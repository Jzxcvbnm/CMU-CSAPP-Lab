#ifndef LINKER_H
#define LINKER_H
#ifdef	__cplusplus
extern "C" {
#endif

/*frist step process of link
 determines the base address of
 each module and the absolute address 
 of each external symbol
 this also can be said as 'Symbol resloution'
*/
extern void process_one();

/*second step process of link
  uses the base addresses and the symbol table 
  computed in pass one to generate the actual 
  output by relocating relative addresses and resolving external references.
  this also can be said 'Relocation'
*/
extern void process_two();

/* link process*/
extern void link(int, const char *);

/*if you want to add additional functons,  you can declare here*/

#ifdef	__cplusplus
}
#endif


#endif
