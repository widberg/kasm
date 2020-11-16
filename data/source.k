entry() : u8
{
	x : u8;
	x = 2;
	return 10 + test(x);
}

test(x : u32) : u32
{
	return x + 5;
}
