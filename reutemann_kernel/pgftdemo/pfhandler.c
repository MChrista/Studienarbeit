#define PRESENT_BIT 1
#define NOT_PRESENT_BIT 0
#define RW_BIT 2
#define OFFSET_KERNEL_PT 0
#define OFFSET_PROGRAMM_PT 32
#define OFFSET_STACK_PT 1023
#define RW_BIT 2


/*
 * Declaration of Page Directory and Page tables
 */
unsigned long page_directory[1024] __attribute__((aligned(0x1000)));
unsigned long kernel_page_table[1024] __attribute__((aligned(0x1000)));
unsigned long programm_page_table[1024] __attribute__((aligned(0x1000)));
unsigned long stack_page_table[1024] __attribute__((aligned(0x1000)));
//General Parameters
int startaddress = 0x200000; //Startaddress for Physical Memory
int page_counter = 0;
const int maxNumberOfPages = 4; //Maximum Number of Pages
unsigned long page_bitfield[1024][32];
//Page replace parameters
int replace_pde_offset = 0;
int replace_pte_offset = 512;


typedef struct pg_struct {
    unsigned long ft_addr;  // faulting linear memory address
    unsigned long     pde;  // Page Directory Entry
    unsigned long     pte;  // Page Table Entry
    unsigned long     off;  // Page Offset
    unsigned long ph_addr;  // Physical Address
    unsigned long   flags;  // Flags = TBD
} pg_struct_t;


static pg_struct_t pg_struct;
int setPresentBit(int pde_offset, int pte_offset, int bool);
int isPresentBit(int pde_offset, int pte_offset);


pg_struct_t *
pfhandler(unsigned long ft_addr)
{
    int page_dir_offset = (ft_addr >> 22) & 0x3FF;
    int page_table_offset = (ft_addr & 0x003FF000) >> 12;
    
    pg_struct.ft_addr = ft_addr;
    pg_struct.pde     = page_dir_offset;
    pg_struct.pte     = page_table_offset;
    pg_struct.off     = ft_addr & 0x00000FFF;
    if( (page_directory[page_dir_offset] & PRESENT_BIT) == PRESENT_BIT ){ //if present Bit is set

        unsigned long *page_table;
        page_table = (unsigned long *)(page_directory[page_dir_offset] & 0xFFFFF000);
/*
        if(IS_PRESENT(*(page_table + page_table_offset) == 0)){
            printf("Makro sagt present\n");
        }else{
            printf("Makro sagt nicht present\n");
        }
*/
        if((*(page_table + page_table_offset) & PRESENT_BIT) != PRESENT_BIT){ //if present Bit is not set
            if(page_counter < maxNumberOfPages){
                 //=  *(page_table + page_table_offset) & 0xFFFFF000;
          
                unsigned long next_address = (unsigned long)(startaddress + page_counter++ * 0x1000 + PRESENT_BIT + RW_BIT);
                *(page_table + page_table_offset) = next_address;
                setPresentBit(page_dir_offset,page_table_offset,1);
                pg_struct.ph_addr = next_address & 0xFFFFF000;
   // Get next free Page and return new virtual Adress
            }else{
                //Get physical address of page you want to replace
                do{
                    //printf(".\n");
                    replace_pte_offset++;
                    if(replace_pte_offset > 1023){
                        if(replace_pde_offset==1023){
                            replace_pde_offset=0;
                            replace_pte_offset=512;//Do not remove Kernel Pages
                        }else{
                            replace_pte_offset =0;
                            replace_pde_offset++;
                        }
                        
                    }
                }while(!isPresentBit(replace_pde_offset,replace_pte_offset));
     
                //Get page table, in which is the page you want to replace and get the physical address of this page
                unsigned long *temp_page_table;
                temp_page_table = (unsigned long *)(page_directory[replace_pde_offset] & 0xFFFFF000);
                unsigned long replace_phy_address = *(temp_page_table + replace_pte_offset) & 0xFFFFF000;

                //Remove old page
                *(temp_page_table + replace_pte_offset) &= 0x0;
                setPresentBit(replace_pde_offset,replace_pte_offset,0);
                
                /*Map new page
                 * Page Table is already present
                 */
                *(page_table + page_table_offset) = (replace_phy_address + RW_BIT + PRESENT_BIT);
                pg_struct.ph_addr = replace_phy_address && 0xFFFFF000;
                
                //Now set Present Bit in bitmap matrix
                setPresentBit(page_dir_offset,page_table_offset,1);
                
            }
        }
        pg_struct.flags = *(page_table + page_table_offset) && 0x00000FFF;
    }else{
        pg_struct.ph_addr = -1;
        pg_struct.flags = 0x0;
    }
    return &pg_struct;
} /* end of pfhandler */

int setPresentBit(int pde_offset, int pte_offset, int bool){
    if(pde_offset < 0 || pde_offset > 1023 || pte_offset < 0 || pte_offset > 1023){
        return 0;     
    }else{
        int index = pte_offset/32;
        if(bool == NOT_PRESENT_BIT){
            page_bitfield[pde_offset][index] &= (~(PRESENT_BIT << (pte_offset%32)));
        }else{
            page_bitfield[pde_offset][index] |= (PRESENT_BIT << (pte_offset%32));
        }
        return 1;
    }
}

int isPresentBit(int pde_offset, int pte_offset){
    if(pde_offset < 0 || pde_offset > 1023 || pte_offset < 0 || pte_offset > 1023){
        return 0;     
    }else{
        int index = pte_offset/32;
        return ((page_bitfield[pde_offset][index] & (PRESENT_BIT << (pte_offset%32)) ) != NOT_PRESENT_BIT);
    }
}

unsigned long* init_paging() {

    // Initialize Page Directory
    
    //Set Directory to blank
    for(int i=0; i<1024;i++){
        *(page_directory+i) = *(page_directory+i) & 0x00000000;
    }
    
    //set Bitfield to blank
    for(int i=0; i<1024;i++){
        for(int j=0; j<32;j++){
            page_bitfield[i][j]=0;
        }
    }
    
    //Copy Kernel to First Page Table
    //for the first MB
    for(int i = 0; i<256; i++){
       kernel_page_table[i] = (unsigned long)(i * 0x1000 + 3);
       setPresentBit(0,i,1);
    }
    *(page_directory + OFFSET_KERNEL_PT) = (unsigned long)kernel_page_table | PRESENT_BIT | RW_BIT;
    *(page_directory + OFFSET_PROGRAMM_PT) = (unsigned long)programm_page_table | PRESENT_BIT | RW_BIT;
    *(page_directory + OFFSET_STACK_PT) = (unsigned long)stack_page_table | PRESENT_BIT | RW_BIT;
   
    //printf("Address of page Directory %p\n",page_directory);
    return page_directory;
}


