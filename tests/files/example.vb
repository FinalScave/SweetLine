// Visual Basic sample code
Option Strict On
Imports System
Imports System.Collections.Generic
Imports System.Linq
Imports System.Threading.Tasks

#Region "Definitions"
#If DEBUG Then
#Const MAX_RETRIES = 3
#End If

Namespace Demo.App

    Public Enum Priority
        Low = 0
        Medium
        High
    End Enum

    Public Delegate Sub Callback(sender As Object, msg As String)
    Public Delegate Function Predicate(Of T)(item As T) As Boolean

    Public Interface IRepository(Of T As {Class, New})
        Function FindAsync(id As Integer) As Task(Of T)
        Sub Save(entity As T)
    End Interface

    <Serializable, Obsolete("Use TaskV2")>
    Public Class TaskItem
        Implements IComparable(Of TaskItem)
        Private _priority As Priority
        Public Property Name As String
        Public Property IsComplete As Boolean

        Public ReadOnly Property Summary As String
            Get
                Return $"{Name} [{_priority}]"
            End Get
        End Property

        Public Sub New(name As String, priority As Priority)
            Me.Name = name
            _priority = priority
        End Sub

        Public Function CompareTo(other As TaskItem) As Integer Implements IComparable(Of TaskItem).CompareTo
            Return _priority.CompareTo(other._priority)
        End Function

        ''' <summary>Execute the task.</summary>
        Public Overridable Sub Execute()
            If Name Is Nothing OrElse Name.Length = 0 Then
                Throw New ArgumentException("Name required")
            End If
            IsComplete = True
        End Sub

        Public Function Format(width As Integer, Optional verbose As Boolean = False) As String
            Return If(verbose, Summary, Name).PadRight(width)
        End Function
    End Class

    Public Class Manager(Of T As {TaskItem, New})
        Implements IRepository(Of T)
        Private ReadOnly _items As New List(Of T)

        Public Sub Save(entity As T) Implements IRepository(Of T).Save
            SyncLock Me
                _items.Add(entity)
            End SyncLock
        End Sub

        Public Async Function FindAsync(id As Integer) As Task(Of T) Implements IRepository(Of T).FindAsync
            Await Task.Delay(10)
            Return _items.FirstOrDefault(Function(x) x.Id = id)
        End Function

        Public Iterator Function HighPriority() As IEnumerable(Of T)
            For Each item As T In _items
                If item.CompareTo(New T()) > 0 Then Yield item
            Next
        End Function
    End Class

    Public Structure Point
        Public X As Double
        Public Y As Double
        Public Sub New(x As Double, y As Double)
            Me.X = x
            Me.Y = y
        End Sub
        Public Shared Operator +(a As Point, b As Point) As Point
            Return New Point(a.X + b.X, a.Y + b.Y)
        End Operator
    End Structure

#End Region

    Public Class Program
        Public Event TaskDone As Callback
        Private WithEvents _timer As Timers.Timer

        Public Shared Sub Main(args() As String)
            Dim hex As Integer = &HFF
            Dim bin As Long = &B1100_1010L
            Dim pi As Double = 3.14R
            Dim price As Decimal = 29.99D
            Dim flag As Boolean = True
            Dim empty As String = Nothing
            Dim ch As Char = "A"c
            Const MAX As Integer = 100

            Dim mgr As New Manager(Of TaskItem)()
            Dim t1 As New TaskItem("Build", Priority.High)
            mgr.Save(t1)

            Dim result As Integer? = If(flag, 42, Nothing)
            If result.HasValue AndAlso result.Value > 0 Then
                Console.WriteLine($"Result: {result.Value}")
            ElseIf result Is Nothing Then
                Console.WriteLine("Nothing")
            Else
                Console.WriteLine("Zero")
            End If

            Select Case t1.Name
                Case "Build" : Console.Write("building")
                Case "Test", "Deploy" : Console.Write("testing")
                Case Else : Console.Write("other")
            End Select

            For i As Integer = 0 To MAX Step 2
                If i Mod 10 = 0 Then Continue For
                If i > 50 Then Exit For
            Next

            Do While flag
                flag = False
            Loop

            ' LINQ and lambda
            Dim items = New List(Of String) From {"Alpha", "Beta", "Gamma"}
            Dim query = From item In items
                        Where item.Length > 4
                        Order By item
                        Select item.ToUpper()
            Dim mapped = items.Select(Function(x) x.Length)
            Dim action = Sub(msg As String) Console.WriteLine(msg)

            ' Type conversions
            Dim obj As Object = 42
            If TypeOf obj Is Integer Then
                Dim num = CInt(obj)
                Dim typed = DirectCast(obj, Integer)
                Dim safe = TryCast(obj, String)
                Dim tName = NameOf(TaskItem)
                Dim gType = GetType(Integer)
            End If

            ' Error handling
            Try
                Using reader As New IO.StreamReader("data.txt")
                    Dim line = reader.ReadLine()
                End Using
            Catch ex As Exception When ex.Message IsNot Nothing
                Console.Error.WriteLine(ex.Message)
            Finally
                Console.WriteLine("Done")
            End Try

            With t1
                .Name = "Rebuild"
                .Execute()
            End With

            REM Legacy comment style
            Dim p = New Point(1.0, 2.0) + New Point(3.0, 4.0)
            Console.WriteLine($"Point=({p.X}, {p.Y})")
        End Sub
    End Class

End Namespace
