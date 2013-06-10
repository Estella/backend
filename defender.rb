require 'redis'

r = Redis.new

counts = Hash.new(0)
r.keys("*:*:count").each do |data|
  offender = data.split(":").first
  blacklist = r.get("#{offender}:repsheet:blacklist") == "true" ? true : false
  next if blacklist
  score = r.get(data).to_i
  counts[offender] += score
end

if counts.size > 0
  blacklist = counts.drop_while {|k,v| v < 20}
  puts "Blacklisting the following offenders"
  blacklist.each do |offender, count|
    puts "  #{offender} (#{count} offenses)"
    r.set("#{offender}:repsheet:blacklist", "true")
  end
else
  puts "There are no new offenders to block"
end
