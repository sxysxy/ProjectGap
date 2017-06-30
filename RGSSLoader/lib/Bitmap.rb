class Bitmap
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
  
  def fill_rect(*arg)
    return if disposed?
    if(arg.size == 2)
      __fill_rect_2args(*arg)
    elsif(arg.size == 5)
      __fill_rect_5args(*arg)
    else
      raise ArgumentError, "Bitmap.new requests 2 or 5 arguments"
    end
  end
end