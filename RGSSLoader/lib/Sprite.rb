#Sprite的重新实现
#   by HfCloud. 
#   repo: https://github.com/sxysxy/ProjectGap
class Sprite
  def initialize(*arg)
    if(arg.size == 0)
      __init
    elsif(arg.size == 1)
      __init_viewport(*arg)
    else
      raise ArgumentError, "Sprite.new requests 0 or 1 arguments" 
    end
  end
end
