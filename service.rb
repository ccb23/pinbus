#!/usr/bin/env ruby

require 'uri'
require 'net/http'

@data = []

# uri = URI('http://localhost:3000/events')
uri = URI('http://pinpirate.herokuapp.com/events')

Net::HTTP.start(uri.host, uri.port) do |http|
  STDIN.each_byte do |char|
    if char == 255
      unless @data.empty?
        query = { 'event[data][]' => @data, 'event[time]' => Time.now.to_i }
        response = http.post(uri.path, URI.encode_www_form(query))
        puts response.body

        @data = []
      end
    else
      @data << sprintf("%.2x", char)
    end
    sleep 0.05
  end
end
