// https://github.com/ToyoDAdmiral/xenu
_BOOL8 __fastcall do_auth(const char *password_file, const char *password)
{
  char buf[12]; // [rsp+10h] [rbp-10h] BYREF
  int fd; // [rsp+1Ch] [rbp-4h]

  buf[8] = 0;
  if ( !strcmp(password, s2) )
    return 1LL;
  fd = open(password_file, 0);
  read(fd, buf, 8uLL);
  return strcmp(password, buf) == 0;
}
