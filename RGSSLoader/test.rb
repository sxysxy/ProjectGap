module Kernel
  def first_scene
    Scene_Title
  end
end

class Scene_TestX < Scene_Base
  def start
    @w = Window_Base.new(0, 0, 272, 208)
    @w.draw_text(4, 4, 200, 100, "233")
  end
  def update
    super
   # p @w.contents_sprite.bitmap == @w.contents
    
  end
end
