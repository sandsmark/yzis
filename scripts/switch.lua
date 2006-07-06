-- Author : Mickael Marchand <mikmak@yzis.org>


function switchHeaderImpl() 
	local str = filename()
	a = string.find(str, "\.%w*$");
	local fname = string.sub(str,0,a-1)
	local fext = string.sub(str,a+1,string.len(fname)-a)
	
	if fext == 'cpp' or fext == 'cc' or fext == 'c' then 
		edit(fname..".h")
	end
	if fext == 'h' then
		edit(fname..".cpp")
	end
end 

map(",m", "<Script>switchHeaderImpl()")


