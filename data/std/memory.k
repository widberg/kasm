memcpy(src : u8*, dst : u8*, size : u32) : void
{
	for (i : u32 = 0; i < size; i = i + 1)
	{
		dst[i] = src[i];
	}
}
