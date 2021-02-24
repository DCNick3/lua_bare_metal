local function main()
    print("Let's do some math")

    local a = {{12, 19}, {14, 17}, {15, 23}, {14, 89}}

    local n = -1
    local cont = true

    while cont do
        n = n + 1
        cont = false
        for _, v in ipairs(a) do
            if n % v[2] ~= v[1] then
                cont = true
            end
        end
    end

    print(n)
end

local r, e = pcall(main)

if not r then
   print("Error: "..tostring(e))
else
   print("All's ok!")
end


