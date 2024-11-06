import struct, hashlib
with open("ota.bin", mode="br") as f:
    f.seek(0x0C)
    codelen = struct.unpack("i", f.read(4))[0] - 512
    f.seek(0x600)
    print("".join(format(x, "02x") for x in hashlib.sha256(f.read(codelen)).digest()))
