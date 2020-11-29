entry() : s32
{
	file : u32 = fopen("source.k", 1);
	x : u32;
	x = 2;
	for(i : u32 = 1; i <= 100; i = i + 1)
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
	fclose(file);
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

fopen(path : u32, mode : u32) : u32
{
	asm
	{
		li $v0, 9
		sys
		copy $t0, $v0
	};
}

fclose(id : u32) : void
{
	asm
	{
		li $v0, 10
		sys
	};
}
