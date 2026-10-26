Import("env")

fpu_flags = ["-mfpu=fpv5-d16", "-mfloat-abi=hard"]
env.Append(LINKFLAGS=fpu_flags)
