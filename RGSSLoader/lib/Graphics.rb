module Graphics
  def self.transition(*arg)
    if arg.size == 0
      __transition_noarg
    elsif arg.size == 1
      __transition_1arg(*arg)
    elsif arg.size == 2
      __transition_2args(*arg)
    elsif arg.size == 3
      __transition_3args(*arg)
    else
      raise ArgumentError, "Graphics.transition requests 0...3 arguments"
    end
  end
end