def common_prefix(left, right)
  i = 0;
  while(i < left.length and i < right.length and left[i] == right[i])
    i += 1
  end
  return left[0..i-1]
end

def find_prefix(text, lst)
  if lst == nil
    return nil
  else
    prefix = text
    lst.each{|i|
      res = common_prefix(prefix, i)
      if res then
        prefix = res
      end
    }
    return prefix
  end
end

def longest_prefix(lst)
  find_prefix(lst[0], lst[1..-1])
end

class Tree
  attr_accessor :letters
  attr_accessor :text
  
  def initialize(lst)
    if lst.length == 1 then
      @letters = nil
    else
      @letters = {}
      "abcdefghijklmnopqrstuvwxyz/?-'_'.0123456789".each_byte{|c| @letters[c] = nil }

      @letters.each_key{|c|
        puts
        puts lst.inspect
        newlst = lst.find_all{|i| i != nil and i[0] == c}.map{|i| i[1..-1]}
        puts newlst
        if not newlst.empty? then
          text = longest_prefix(newlst)
          # puts text
          @letters[c] = Tree.new(lst.find_all{|i| i != nil}.map{|i| i[text.length..-1]})
        end
      }
    end
  end
end


lst = File.new("indices/vactrax.pak").readlines().each{|i| i.chop!}.find_all{|i| not i.empty?}.sort

lst.sort!
# find_prefix(lst[0], lst[1..-1])
Tree.new(lst)

# EOF #
