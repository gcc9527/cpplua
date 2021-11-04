




local lfs = require("lfs")

for val in lfs.dir("../Protobuf") do
    if val ~= "." and val ~= ".." then
        gMainThread:parseProto(val)
    end
end