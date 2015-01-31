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
	if @data[0] == '05' and @data[1] == '0c'
	  print @data[2..-1].join('') + '  -  '
	end
        query = { 'event[data][]' => @data, 'event[time]' => Time.now.to_i }
        response = http.post(uri.path, URI.encode_www_form(query))
        puts response.code #body
 
        @data = []
      end
    else
      @data << sprintf("%.2x", char)
    end
    sleep 0.02
  end
end
