struct output_stream
{
	stream_id : u32;
};

const system_out : struct output_stream = { 0 };

struct input_stream
{
	stream_id : u32;
};

const system_in : struct input_stream = { 0 };

fprintf(out : struct output_stream*, format_string : u8*, ...) : void
{
	args : u8* = &format_string + 1;
	for (c : u8* = format_string; @c != '\0'; c = c + 1)
	{
		switch (@c)
		{
		case 'c':
			char := @(u8*)args;
			args = args + 1;
			break;
		case 'd':
			int := @(s32*)args;
			args = args + 4;
			break;
		case 'u':
			int := @(u32*)args;
			args = args + 4;
			break;
		default:
			putc(out, @c);
			break;
		}
	}
}

fscanf(in : struct input_stream*, format_string : u8*, ...) : u32
{
	
}
