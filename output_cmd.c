/* cmd.c TRS-DOS /CMD output driver for vasm */
/* (c) 2026 Bryan Woodruff, Cortexa LLC */

#include "vasm.h"

#ifdef OUTCMD
static char *copyright="vasm TRS-DOS /CMD output module 1.0 (c) 2026 Bryan Woodruff, Cortexa LLC";

static char *exec_symname;
static taddr exec_addr;

/* TRS-DOS /CMD format:
 * Load Block:
 *   00:    0x01 (load block flag)
 *   01-02: block length including this header (little endian)
 *   03-04: load address (little endian)
 *   05-xx: data bytes
 *
 * Transfer Block (end of file):
 *   00:    0x02 (transfer block flag)
 *   01-02: 0x0002 (block length = 2)
 *   03-04: execution address (little endian)
 */

static int orgcmp(const void *sec1,const void *sec2)
{
  if (((utaddr)(*(section **)sec1)->org) > ((utaddr)(*(section **)sec2)->org))
    return 1;
  if (((utaddr)(*(section **)sec1)->org) < ((utaddr)(*(section **)sec2)->org))
    return -1;
  return 0;
}


static void write_output(FILE *f,section *sec,symbol *sym)
{
  section *s,**seclist,**slp;
  unsigned long long pc;
  size_t nsecs;
  atom *p;
  taddr segsize;

  if (sec == NULL)
    return;

  /* find execution address from symbol or END directive */
  for (; sym; sym=sym->next) {
    if (sym->type==IMPORT)
      output_error(6,sym->name);  /* undefined symbol */

    if (exec_symname!=NULL && !strcmp(exec_symname,sym->name)) {
      exec_addr = sym->pc;
      exec_symname = NULL;  /* found the start-symbol */
    }
  }
  if (exec_symname != NULL)
    output_error(6,exec_symname);  /* start-symbol not found */

  /* count sections and check for overlaps */
  nsecs = chk_sec_overlap(sec);

  /* make array of section pointers, sorted by start address */
  seclist = (section **)mymalloc(nsecs * sizeof(section *));
  for (s=sec,slp=seclist; s!=NULL; s=s->next) {
    /* skip unallocated sections */
    if (s->flags & UNALLOCATED)
      continue;
    /* skip empty sections */
    if (s->pc == s->org)
      continue;
    *slp++ = s;
  }
  nsecs = slp - seclist;  /* actual sections with content */

  if (nsecs > 1)
    qsort(seclist,nsecs,sizeof(section *),orgcmp);

  if (nsecs == 0) {
    /* no sections with content */
    myfree(seclist);
    return;
  }

  sec = *seclist;  /* first section */

  /* write each section as a load block */
  for (slp=seclist; slp<seclist+nsecs; slp++) {
    s = *slp;
    segsize = s->pc - s->org;

    /* TRS-DOS /CMD load block header */
    fw8(f,0x01);                        /* load block flag */
    fw16(f,segsize+5,0);                /* block length (includes 5-byte header) */
    fw16(f,s->org,0);                   /* load address (little endian) */

    /* write section contents */
    for (p=s->first,pc=(unsigned long long)s->org; p; p=p->next) {
      unsigned long long npc;

      npc = fwpcalign(f,p,s,pc);

      if (p->type == DATA)
        fwdblock(f,p->content.db);
      else if (p->type == SPACE)
        fwsblock(f,p->content.sb);

      pc = npc + atom_size(p,s,npc);
    }
  }

  /* TRS-DOS /CMD transfer block (execution address) */
  fw8(f,0x02);                          /* transfer block flag */
  fw16(f,0x0002,0);                     /* block length = 2 */
  fw16(f,exec_addr?exec_addr:sec->org,0);  /* execution address (little endian) */

  myfree(seclist);
}


static int output_args(char *p)
{
  if (!strncmp(p,"-exec=",6)) {
    exec_symname = p+6;
    return 1;
  }
  return 0;
}


int init_output_cmd(char **cp,void (**wo)(FILE *,section *,symbol *),
                    int (**oa)(char *))
{
  *cp = copyright;
  *wo = write_output;
  *oa = output_args;
  exec_addr = 0;
  exec_symname = NULL;
  return 1;
}

#else

int init_output_cmd(char **cp,void (**wo)(FILE *,section *,symbol *),
                    int (**oa)(char *))
{
  return 0;
}

#endif
