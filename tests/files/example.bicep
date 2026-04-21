#disable-next-line no-unused-vars BCP335
targetScope = 'resourceGroup'

metadata owner = {
  name: 'Platform Engineering'
  contact: 'platform@example.com'
  'support-url': 'https://status.example.com/platform'
}

metadata releaseNotes = '''
Launchpad baseline deployment.
This template provisions web, storage, and monitoring resources for ${deployment().name}.
Use it as a starting point for team environments.
'''

@description('Short service name used as the resource prefix.')
@minLength(3)
@maxLength(18)
param appName string

@description('Deployment environment.')
@allowed([
  'dev'
  'staging'
  'prod'
])
param environmentName string = 'dev'

@description('Primary Azure region for all resources.')
param location string = resourceGroup().location

@description('Optional additional tags.')
param tags object = {}

@description('How many storage accounts should be created for the workload.')
@minValue(1)
@maxValue(4)
param storageCount int = 2

@description('Enable Application Insights and diagnostics wiring.')
param enableMonitoring bool = true

@secure()
param storageAccessRules object = {
  allowIps: [
    '10.20.0.0/24'
    '10.30.0.0/24'
  ]
}

type skuNameType = 'Standard_LRS' | 'Standard_GRS' | 'Standard_ZRS'

type siteConfigType = {
  alwaysOn: bool
  appSettings: {
    name: string
    value: string
  }[]
  ftpsState: 'Disabled' | 'FtpsOnly'
}

func buildStorageName(prefix string, index int) string => '${take(toLower(prefix), 11)}${uniqueString(resourceGroup().id, string(index))}'

var namePrefix = '${appName}-${environmentName}'

var commonTags = union(tags, {
  environment: environmentName
  service: appName
  'app-name': appName
  managedBy: 'bicep'
})

var siteConfig siteConfigType = {
  alwaysOn: environmentName != 'dev'
  ftpsState: 'Disabled'
  appSettings: [
    {
      name: 'ASPNETCORE_ENVIRONMENT'
      value: environmentName
    }
    {
      name: 'FEATURE_FLAGS'
      value: '{"search":true,"billing":false}'
    }
    {
      name: 'WELCOME_BANNER'
      value: 'it''s ${appName} in ${environmentName}'
    }
  ]
}

var runtimeNote = '''
The deployment will create:
- one app service plan
- one web app
- ${storageCount} storage account(s)
- optional monitoring resources
'''

resource appServicePlan 'Microsoft.Web/serverfarms@2023-12-01' = {
  name: '${namePrefix}-plan'
  location: location
  tags: commonTags
  sku: {
    name: environmentName == 'prod' ? 'P1v3' : 'B1'
    tier: environmentName == 'prod' ? 'PremiumV3' : 'Basic'
  }
  properties: {
    reserved: false
  }
}

resource storageAccounts 'Microsoft.Storage/storageAccounts@2023-05-01' = [for i in range(0, storageCount): {
  name: buildStorageName(namePrefix, i)
  location: location
  tags: union(commonTags, {
    role: 'storage'
    ordinal: string(i)
  })
  sku: {
    name: 'Standard_LRS'
  }
  kind: 'StorageV2'
  properties: {
    accessTier: 'Hot'
    allowBlobPublicAccess: false
    minimumTlsVersion: 'TLS1_2'
    networkAcls: {
      bypass: 'AzureServices'
      defaultAction: 'Deny'
      ipRules: [for ip in storageAccessRules.allowIps: {
        action: 'Allow'
        value: ip
      }]
    }
  }
}]

resource insights 'Microsoft.Insights/components@2020-02-02' = if (enableMonitoring) {
  name: '${namePrefix}-appi'
  location: location
  tags: commonTags
  kind: 'web'
  properties: {
    Application_Type: 'web'
    WorkspaceResourceId: ''
  }
}

resource webApp 'Microsoft.Web/sites@2023-12-01' = {
  name: '${namePrefix}-web'
  location: location
  tags: union(commonTags, {
    note: runtimeNote
  })
  properties: {
    serverFarmId: appServicePlan.id
    httpsOnly: true
    siteConfig: {
      alwaysOn: siteConfig.alwaysOn
      ftpsState: siteConfig.ftpsState
      appSettings: concat(siteConfig.appSettings, [
        {
          name: 'APPLICATIONINSIGHTS_CONNECTION_STRING'
          value: enableMonitoring ? insights.properties.ConnectionString : ''
        }
        {
          name: 'PRIMARY_STORAGE_NAME'
          value: storageAccounts[0].name
        }
      ])
    }
  }
}

resource siteExtension 'Microsoft.Web/sites/siteextensions@2023-12-01' = if (enableMonitoring) {
  name: '${webApp.name}/Microsoft.ApplicationInsights.AzureWebSites'
  location: location
}

resource settings 'Microsoft.Web/sites/config@2023-12-01' = {
  name: '${webApp.name}/logs'
  properties: {
    applicationLogs: {
      fileSystem: {
        level: environmentName == 'prod' ? 'Warning' : 'Information'
      }
    }
    detailedErrorMessages: {
      enabled: true
    }
    failedRequestsTracing: {
      enabled: environmentName != 'prod'
    }
    httpLogs: {
      fileSystem: {
        enabled: true
        retentionInDays: 7
        retentionInMb: 64
      }
    }
  }
}

resource existingPlan 'Microsoft.Web/serverfarms@2023-12-01' existing = {
  name: appServicePlan.name
}

module diagnostics './modules/diagnostics.bicep' = if (enableMonitoring) {
  name: '${namePrefix}-diagnostics'
  params: {
    resourceId: webApp.id
    appName: appName
    environmentName: environmentName
    diagnosticName: '${webApp.name}-diag'
  }
}

output serviceUrl string = 'https://${webApp.properties.defaultHostName}'
output storageNames array = [for account in storageAccounts: account.name]
output planName string = existingPlan.name
output monitoringEnabled bool = enableMonitoring
