class Window
  attr_reader :windowskin
  attr_reader :contents
  attr_accessor :cursor_rect
  attr_accessor :viewport
  attr_accessor :active
  attr_accessor :visible
  attr_accessor :arrows_visible
  attr_accessor :pause
  attr_reader :x, :y, :z, :width, :height
  attr_reader :ox, :oy
  attr_accessor :padding, :padding_bottom
  attr_accessor :opacity, :back_opacity
  attr_accessor :contents_opacity
  attr_accessor :openness
  attr_accessor :tone
  
  attr_accessor :__disposed
  
  attr_reader :back_sprite   #显示背景用的精灵
  attr_reader :cursor_sprite #显示光标用的精灵
  attr_reader :frame_sprite  #显示边框用的精灵
  attr_reader :contents_sprite #显示contents内容的精灵
  attr_reader :arrows_sprite #显示箭头用的精灵
  attr_reader :pause_sprite  #显示pause状态用的精灵
  attr_reader :frame_count   #窗口已经更新的帧数
  
  def initialize(*arg)
    move(*arg)
    @frame_count = 0
    __init
  end
  
  def move(x, y, w, h)
    self.x = x; self.y = y
    self.width = w; self.height = h
  end
  def open?
    return @openness >= 255
  end
  def close?
    return @openness <= 0
  end
  def disposed?
    return @__disposed == true
  end
  def dispose
    @back_sprite.dispose
    @cursor_sprite.dispose
    @frame_sprite.dispose
    @contents_sprite.dispose
    @arrows_sprite.dispose
    @pause_sprite.dispose
    @__disposed = true
  end
  def windowskin=(w)
    @windowskin = w
    __refresh_all
  end
  def contents=(c)
    @contents = c
  end
  
  def x=(_x)
    @x = _x.to_i
  end
  def y=(_y)
    @y = _y.to_i
  end
  def z=(_z)
    @z = _z.to_i
  end
  def width=(_w)
    @width = _w.to_i
  end
  def height=(_h)
    @height = _h.to_i
  end
  def ox=(_ox)
    @ox = _ox.to_i
  end
  def oy=(_oy)
    @oy = _oy.to_i
  end
end

class Window_Selectable 
  alias :update_cursor_patch_RGSSPlugin :update_cursor
  def update_cursor
    update_cursor_patch_RGSSPlugin
    __refresh_cursor
  end
end