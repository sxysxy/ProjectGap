Win32API.new("lib/RGSSRuntimePlugin.dll", "ApplyPlugin", "", "").call

t = Bitmap.new(544, 416)
t.fill_rect(0, 0, 544, 416, Color.new(0, 255, 0))

print Graphics.frame_count

loop do
  Input.update
  Graphics.update
  t.show_on_screen(0, 0) #因为精灵还没有实现，先这样偷懒一下，让Bitmap#show_on_screen每帧调用一次
  print Graphics.frame_count
end