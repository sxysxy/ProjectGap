module Kernel
  def first_scene
    Scene_TestX
  end
end
class Window
  alias :xxx :initialize
  def initialize(*arg)
   # RGSSLoader.debug_break
    xxx(*arg)
   # msgbox "stop"
  end
end

class Scene_TestX < Scene_Base
  def start
    @w = Window_Base.new(0, 0, 544, 416)
  #  @s = Sprite.new
  # @s.bitmap = Cache.title1("Book")
  #  @s.z = -100
  end
  def update
    super
    @w.contents.show_on_screen(0, 0)
  end
end
