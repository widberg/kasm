entry() : u8
{
	x : u8;
	x = 2;
	for(i : u8, i = 1; i <= 100; i = i + 1)
	{
		if (i % 3 == 0 && i % 5 == 0)
		{
			print("fizzbuzz\n");
		}
		else if (i % 3 == 0)
		{
			print("fizz\n");
		}
		else if (i % 5 == 0)
		{
			print("buzz\n");
		}
		else
		{
			printi(i);
			print("\n");
		}
	}
	return 10 + test(x);
}

test(x : u32) : u32
{
	return x + 5;
}

printi(x : u32) : void
{
	asm
	{
		li $v0, 2
		sys
	};
}

print(x : u32) : void
{
	asm
	{
		li $v0, 6
		sys
	};
}
