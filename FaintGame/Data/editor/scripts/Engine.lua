Engine = {}

function Engine.Log(message)
	log("[Engine] " .. message)
end

package.path = package.path .. ";data/editor/scripts/?.lua"