#Bitmap的重新实现
#   by HfCloud. 
#   reop: https://github.com/sxysxy/ProjectGap

class Bitmap
  #这里展示了已经实现的接口。
  #对于RGSSVA文档里面已经有的不再复述

  #初始化Bitmap对象
  def initialize(*arg)
    @bitmap_data = nil
    if arg.size == 2
      __init_wh(*arg)
    elsif arg.size == 1
      (raise ArgumentError, "Invalid Argument") if(arg[0].class != String)
      __init_path(*arg)
    else
      raise ArgumentError, "Bitmap.new requests 1..2 arguments"
    end
    (raise RGSSError, "Fail To Create Bitmap") if !__get_texture
  end 
  
  #fill_rect的前台。类似的，由于可变参方法的存在，我们做了几个这样的参数个数判断的处理
  #渲染速度超级快
  def fill_rect(*arg)
    if(arg.size == 2)
      __fill_rect_2args(*arg)
    elsif(arg.size == 5)
      __fill_rect_5args(*arg)
    else
      raise ArgumentError, "Bitmap.fill_rect requests 2 or 5 arguments"
    end
  end
  
  #渲染速度超级快
  def blt(*arg)
    if(arg.size == 4)
      __blt_4args(*arg)
    elsif arg.size == 5
      __blt_5args(*arg)
    else
      raise ArgumentError, "Bitmap#blt requests 4..5 arguments" 
    end
  end
  
  #渲染速度超级快
  def stretch_blt(*arg)
    if arg.size == 3
      __stretch_blt_3args(*arg)
    elsif arg.size == 4
      __stretch_blt_4args(*arg)
    else
      raise ArgumentError, "Bitmap#stretch_blt requests 3..4 arguments" 
    end
  end
  
  #渲染速度超级快
  def clear_rect(*arg)
    if arg.size == 1
      __clear_rect_1args(*arg)
    elsif arg.size == 4
      __clear_rect_4args(*arg)
    else
      raise ArgumentError, "Bitmap#clear_rect requests 1 or 4 arguments" 
    end
  end
  
  #往下开始，注释千万不要去掉
=begin
  def width
  end
  def height
  end
  def clear
  end
  def get_pixel(x, y)
  end
  def set_pixel(x, y, color)
  end
  def hue_change(hue)
    #改变色调，这是个很慢的方法，没比rgss默认的方法快多少
      #求硬件渲染实现这个的方法qvq
  end
  def get_all_pixels
    #返回位图的所有像素点，一个width*height大小的Array，里面是Color
    #这个方法也很慢！如果是频繁屏幕截图Graphics.snap_to_bitmap，得到的位图对象请不要用这个方法
  end
  def show_on_screen(x, y)
    #懒省事专用，不借助精灵，直接把这个位图贴到画面上。(x,y)左上角坐标
    #方法：在场景的update里面调用这个show_on_screen方法即可
  end
=end

  #未实现的
  def draw_text(*arg)  #很快就去实现w
  end
  
  #blur系列，感觉没啥用，我不想写了quq..
  def blur
  end
  def radial_blur(a, b)
  end
  
  def gradient_fill_rect(*arg) #渐变色填充矩形，要我说，干脆提前在PS里面画好渐变色好了= =
                                #这个没想到怎么硬件渲染，会很慢
  end
  
end