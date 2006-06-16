def common_prefix(left, right)
  i = 0;
  while(i < left.length and i < right.length and left[i] == right[i])
    i += 1
  end
  if i > 0
    return left[0..i-1]
  else
    return ""
  end
end

class Tree
  attr_accessor :letters
  attr_accessor :text
  
  def initialize()
    @letters = nil
    @text    = nil
  end

  def get_child(c)
#    puts "CHILD: #{c.inspect}"

    if not c then 
      raise "Broken!"
    end

    child = @letters[c]
    if not child then
      child = Tree.new() 
      @letters[c] = child
    end
    return child
  end
  
  def add(text)
    if not text or text.empty? then
      # ignore
    elsif @letters then
      get_child(text[0]).add(text[1..-1])
    elsif @text then
      if @text == text then
        puts "Error: double entry"
      else
        @letters = {}

        prefix = common_prefix(@text, text)

        puts "Prefix: '#{prefix}'"
        puts "@Text:  '#{@text}' #{@text[prefix.length()-1].chr}"
        puts "Text:   '#{text}'  #{text[prefix.length()-1].chr}"
        
        get_child(@text[prefix.length()]).add(@text[prefix.length()..-1])
        get_child(text[prefix.length()]).add(text[prefix.length()..-1])

        @text = prefix[0..prefix.length()-1]
      end
    else
      @text = text
    end
  end

  def iter(prefix, proc)
    if @letters then
      @letters.each_pair{|k,v|
        #puts "KEYVAL: #{k.inspect} #{v}" # #{@letters.inspect}"
        v.iter(prefix + @text + k.chr, proc)
      }
    elsif @text then
      proc.call(prefix+@text)
    else
      # Empty
    end
  end

  def each(&block)
    iter("", block)
  end
  
  def null?()
    not @letters and not @text
  end
end


lst = File.new("indices/vactrax.pak").readlines().each{|i| i.chop!}.find_all{|i| not i.empty?}.sort
tree = Tree.new()
lst.each{|i| tree.add(i) }

tree.each(){|i| puts i}

# EOF #
