int __cdecl main(int argc, const char **argv, const char **envp)
{
  int v4; // [rsp+1Ch] [rbp-24h] BYREF
  char v5[16]; // [rsp+20h] [rbp-20h] BYREF
  char buf[16]; // [rsp+30h] [rbp-10h] BYREF

  buf[8] = 0;
  v5[8] = 0;
  puts("Username: ");
  read(0, buf, 8uLL);
  read(0, &v4, 1uLL);
  puts("Password: ");
  read(0, v5, 8uLL);
  read(0, &v4, 1uLL);
  v4 = authenticate(buf, v5);
  if ( !v4 )
    rejected(buf);
  return sub_4006ED(buf);
}
