plug = {
	name = 'altcraft',
	displayName = "AltCraft Core Plugin",
	onLoad = nil,
	onUnload = nil,
}

function plug:onLoad ()
	print("Loaded "..self.name.."-plugin!")
end

AC:RegisterPlugin(plug)