# Ruby syntax example for SweetLine
# Visit https://ruby-lang.org and http://example.com/docs for references.

=begin
This block comment also mentions https://example.com/reference.
=end

module SweetLine
  module Formatting
    module_function
    def compact_path(path) = path.to_s.strip.gsub(%r{/+}, "/")
  end

  class Parser < BaseParser
    VERSION = "1.0.0"
    API_ROOT = %q{https://api.example.com/v1}
    REGEX = %r{https?://[A-Za-z0-9._~:/?#\[\]@!$&()*+,;=%-]+}i
    DEFAULTS = {
      timeout: 1.5,
      retries: 3,
      enabled: true,
      mode: :fast,
      headers: %w[Accept Content-Type X-Request-Id],
      roles: %i[reader writer admin],
      "api_url" => "https://service.example.com"
    }

    @@instances = 0
    @@cache = {}

    attr_reader :path, :options
    alias source_path path
    def initialize(path, options = {})
      @path = Formatting.compact_path(path)
      @options = DEFAULTS.merge(options)
      @@instances += 1
      $global_status = :ready
    end

    def []=(key, value)
      @options[key] = value
    end

    def self.build(path) = new(path, mode: :fast, retry_count: 2)

    def self.configure(name, &block)
      define_method("configure_#{name}", &block)
    end
    configure(:headers) do |extra = {}|
      @options[:headers] = (@options[:headers] || {}).merge(extra)
    end

    def call(url = API_ROOT, timeout: DEFAULTS[:timeout])
      uri = URI.parse("#{url}/users/#{@path}")
      header = "GET #{uri} via #{self.class.name}"
      label = :"request_#{@path}"
      query = /id=(\d+)&name=([A-Za-z_]\w*)/
      alt = %r{https://#{Regexp.escape(@path)}/items/\d+}i
      payload = <<~JSON
        {
          "url": "https://service.example.com/#{@path}",
          "status": "ok",
          "count": 42,
          "nested": {"enabled": true, "label": "sweet"}
        }
      JSON
      note = <<-'TEXT'
        https://example.com/plain/#{@path} is plain text here.
      TEXT
      shell = %x[printf "parser:%s" "#{@path}"]
      words = %W[user #{@path} #{timeout}]
      flags = %i[trace verbose dry_run]
      matcher = ->(value) { value.to_s.match?(query) || value.to_s.match?(REGEX) }
      summary = case timeout
      when 0 then :disabled
      when 1..5 then :slow
      else :fast
      end

      return nil if url.nil?

      {
        header: header,
        label: label,
        query: query,
        alt: alt,
        payload: payload,
        note: note,
        shell: shell,
        words: words,
        flags: flags,
        matcher: matcher.call(uri.to_s),
        summary: summary
      }
    rescue URI::InvalidURIError, ArgumentError => e
      warn e.message
      :error
    ensure
      self.class.touch(@path)
    end

    def each_item(items)
      return enum_for(:each_item, items) unless block_given?
      items.each { |item| yield item }
    end
    def fetch!
      begin
        response = call
        raise "empty response" unless response
        response
      rescue RuntimeError => e
        { error: e.message, retryable: true }
      ensure
        @options[:last_fetch] = Time.now.utc
      end
    end

    def normalize(value)
      case value
      when String then value.strip
      when Symbol then value.to_s
      when Numeric then format("%.2f", value)
      when nil then "nil"
      else value.inspect
      end
    end
    class << self
      def touch(path)
        @@cache[path] ||= []
        @@cache[path] << Time.now.utc
      end
      def instances = @@instances
      def cache = @@cache
    end
  end

  class Report
    def initialize(parser)
      @parser = parser
    end

    def render(items)
      lines = items.map.with_index { |item, index| "#{index}: #{@parser.normalize(item)}" }
      <<~REPORT
        Report for #{@parser.source_path}
        #{lines.join("\n")}
      REPORT
    end
  end
end
