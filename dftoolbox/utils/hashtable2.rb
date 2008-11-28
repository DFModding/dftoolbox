def common_prefix(left, right)
  i = 0;
  while(i < left.length and i < right.length and left[i] == right[i])
    i += 1
  end

  return left[0...i]
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
  if lst.length == 1 then
    lst[0]
  else
    find_prefix(lst[0], lst[1..-1])
  end
end

def split_by_letter(lst)
  letters = {}
  lst.each{|line| letters[line[0]] = nil }

  lst2 = []
  letters.each_key{|key|
    chunk = []

    lst.each{|line| 
      if line[0] == key  then
        chunk.push(line[1..-1])
      end
    }
    lst2.push([key, chunk])
  }
  return lst2
end

lst = File.new("indices/vactrax.pak").readlines().each{|i| i.chop!}.find_all{|i| not i.empty?}.sort

def build_tree(lst)
  split_by_letter(lst).map{|c, el| 
    if not el or el.empty? then
      nil
    else
      #puts "El: '#{c}' #{el.inspect}"

      prefix = longest_prefix(el)

      if prefix then
        newlst = el.map{|line| line[prefix.length..-1] }

        #puts "Prefix: #{prefix}"
        #newlst.each{|j| puts "newlst: #{j.inspect}"}
        #puts 

        if newlst.length() == 1 then
          [c, prefix, nil]
        else
          [c, prefix, build_tree(newlst)]
        end
      else
        [c, prefix, nil]
      end
    end
  }
end

tree = build_tree(lst)
# puts tree.inspect

def print_tree(global_prefix, global_tree)
  global_tree.each{|c, prefix, tree|
    if tree then 
      print_tree(global_prefix + c.chr + prefix, tree)
    else
      puts global_prefix + c.chr + prefix  
    end
  }
end

def print_chunks(tree)
  tree.each{|c, prefix, tree|
    if tree then 
      print_chunks(tree)
    else
      puts prefix  
    end
  }  
end

print_chunks(tree)
# print_tree("", tree)

# EOF #
