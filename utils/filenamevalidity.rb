$script_pattern = File.new("script-pattern.txt").readlines().map{|i| i.chop + "_[0-9a-fA-F]+\\.png" }

$dirs = {}
File.new("dreamfall-directories.txt").readlines().each{|i|
  $dirs[i.chop] = true
}


$files = {}
File.new("dreamfall-files2.txt").readlines().each{|i|
  $files[i.chop] = true
}

def check_validity2(filenames)
  good = []
  filenames.each{|i|
    if $files.has_key?(i) then
      good.push(i)
    end
  }

  if good.empty?
    filenames.each{|i| puts "Good: #{i}"}
  else
    filenames.each{|i| puts "Missing: #{i}"}
  end
  puts
end

def check_validity(filenames)
  if filenames.length == 1 then
    #puts "Good:    #{filenames}"
    puts filenames[0]
  else
    good    = []
    maybe   = []
    unknown = []
    bad     = []
    discard = []

    filenames.each{|i|
      if $files.has_key?(i) then
        good.push(i)
      else
        case i[-4..-1] 
        when ".lip", ".mp3"
          good.push(i)
        else
          #          if i.match(/^data\//) then
          #            nil # ignore
          if not i.match(/\.[a-z][a-z][a-z]$/) then
            discard.push(i)
          elsif i.match(/^data\/scripts\/script_[a-z-]+_[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f].[a-z][a-z][a-z]$/) then
            good.push(i)
          elsif i.match(/_[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f].cdr$/) then
            maybe.push(i)
          elsif i.match(/_[0-9a-f][0-9a-f][0-9a-f][0-9a-f].[a-z][a-z][a-z]$/) then
            maybe.push(i)
          elsif i.match(/^\/art\/shaders\/prog\/d3d9\/vivid_d/) then
            maybe.push(i)
          elsif i.match(/8ic/) then
            bad.push(i)
          elsif i.match(/anim_win_xightmaps/) then
            bad.push(i)
          elsif i.match(/\/-/) or i.match(/-\//) or i.match(/\/_/) or i.match(/_\//) then
            bad.push(i)
          elsif i.match(/\/[0-9]/) then
            bad.push(i)
          elsif i.match(/_[0-9a-f][0-9a-f][0-9a-f].[a-z][a-z][a-z]$/) then
            maybe.push(i)
          elsif i.match(/[a-z][0-9][a-z]/) then
            bad.push(i)
          elsif i.match(/\..*\./) then
            bad.push(i)
          elsif i.match(/[?']/) then
            bad.push(i)
          elsif i.match(/lightmaps\/.*lightmap/) then
            maybe.push(i)
          elsif i.match(/_nml.png$/) then
            maybe.push(i)
          elsif $dirs.has_key?(i.gsub(/\/[^\/]+$/, ''))
            maybe.push(i)
          else
            unknown.push(i)
          end
        end
      end
    }


    if good.empty? and maybe.length == 1 then
      good = maybe
    end

    if good.empty? and unknown.length == 1 then
      good = unknown
    end

    if good.length != 1 then
      maybe = maybe + good
      good = []
    end

    if good.length == 1
      puts good[0]
    end

    if false then
      if not good.empty? then
        good.each   {|i| puts "Good:    #{i}"}
      elsif not maybe.empty?
        maybe.each  {|i| puts "Maybe:   #{i}"}
      elsif not unknown.empty?
        unknown.each{|i| puts "Unknown: #{i}"}
      else
        bad.each    {|i| puts "Bad:     #{i}"}
      end
    end

    puts ""
  end
end

ARGV.each{|args|
  filenames = []
  File.new(args).readlines().each{|i| i.chop!
    if i == "" then
      check_validity(filenames)
      filenames = []
    else
      filenames.push(i)
    end
  }
}
