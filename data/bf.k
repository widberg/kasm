%include "stl.k"

entry() : u32
{
	pc : u32 = 0;
	ptr : u32 = 0;
	scope : u32 = 0;

	programFile := file_open("source.bf", "r");
	program := vector(u8, file_get_size(programFile));
	file_read(programFile, vector_get_data(program), file_get_size(programFile));

	while (pc < vector_get_size(program))
	{
		switch (*vector_at(program, pc))
		case('>')
		{
			ptr++;
		}
		case('<')
		{
			ptr--;
		}
		case('+')
		{
			*vector_at(memory, ptr) = *vector_at(memory, ptr) + 1;
		}
		case('-')
		{
			*vector_at(memory, ptr) = *vector_at(memory, ptr) - 1
		}
		case('.')
		{
			printf("%c", *vector_at(memory, ptr));
		}
		case(',')
		{
			scanf("%c", vector_at(memory, ptr));
		}
		case('[')
		{
			scope++;
			if (!*vector_at(memory, ptr))
			{
				prescope := scope;
				pc++;
				while (pc < vector_get_size(program))
				{
					switch (*vector_at(program, pc))
					{
					case '[':
						scope++;
						break;
					case ']':
						scope--;
						break;
					}

					if (prescope == scope)
					{
						break;
					}
					else
					{
						pc++;
					}
				}
			}
		}
		case(']')
		{
			scope--;
			if (*vector_at(memory, ptr))
			{
				prescope := scope;
				pc--;
				while (pc < vector_get_size(program))
				{
					switch (*vector_at(program, pc))
					{
					case '[':
						scope++;
						break;
					case ']':
						scope--;
						break;
					}

					if (prescope == scope)
					{
						break;
					}
					else
					{
						pc--;
					}
				}
			}
		}
		pc++;
	}

	return 0;
}
