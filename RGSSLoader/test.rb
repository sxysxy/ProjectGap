Win32API.new("lib/RGSSRuntimePlugin.dll", "ApplyPlugin", "", "").call
Font.default_name = "simhei.ttf"

t = Bitmap.new(544, 416)
t.fill_rect(0, 0, 544, 416, Color.new(0, 255, 0))

t.font.color = Color.new(0, 0, 0, 255);
t.font.strike_through = true
t.draw_text(100, 100, 400, 100, "当然是原谅啊")

loop do
  Input.update
  Graphics.update
  t.show_on_screen(0, 0) #因为精灵还没有实现，先这样偷懒一下，让Bitmap#show_on_screen每帧调用一次
end
