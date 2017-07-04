#Sprite的重新实现
#   by HfCloud. 
#   repo: https://github.com/sxysxy/ProjectGap

class Sprite
  
  attr_accessor :vmirror #新增特性：精灵是否上下翻转的标记
  def init_default_members
    @bitmap = nil
    @viewport = nil
    @visible = true
    @x = @y = @z = @ox = @oy = 0
    @zoom_x = @zoom_y = 1.0
    @angle = 0
    @mirror = false
    @vmirror = false
    @bush_deptu = 0
    @bush_opacity = 128
    @opacity = 255
    @blend_type = 0
    @tone = Tone.new(0, 0, 0, 0)
    @disposed = false
  end
  def initialize(*arg)
    init_default_members
    if arg.size == 1
      @viewport = arg[0]
    elsif arg.size > 1
      raise ArgumentError, "Sprite.new requests 0 or 1 arguments" 
    end
    __init
  end
  def bitmap
    return @bitmap
  end
  def bitmap=(bmp)
    @bitmap = bmp
  end
  attr_accessor :visible
  attr_accessor :x, :y, :ox, :oy, :zoom_x, :zoom_y
  attr_accessor :angle
  attr_accessor :mirror, :vmirror
  
  
  #attr_accessor :angle, :mirror  
  def dispose
    @disposed = true
  end
  def disposed?
    @disposed == true
  end
end
