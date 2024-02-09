#include <elf.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <elf.h>

#include <string.h>

// 打印整个ELF文件
void printfElf();
// 打印ELF Header
void printfElfHeader(Elf64_Ehdr* header);
// 打印ELF所有Section Headers
void printfSectionHeaders(Elf64_Ehdr* header);
// 打印ELF指定的Section Header
void printfSectionHeader(Elf64_Shdr* header,int index);
// 打印Section内容
void printfSection(Elf64_Ehdr*,Elf64_Shdr*);
// 打印symbol table (.symtab & .dynsym section)
void printfSymTabSection(Elf64_Ehdr* ehdr, Elf64_Shdr* shdr);
// 打印rela table
void printfRelaTabSection(Elf64_Ehdr* ehdr, Elf64_Shdr* shdr);
// 打印phdr
void printfProgramHeader();
// 打印.dynamic Section
void printfDynamic();
// 初始化全局遍历
void initGlobal(char* buf,int len);

// ELF header
Elf64_Ehdr* ehdr = NULL;
// Section Header
Elf64_Shdr* shdr = NULL;
// Program Header
Elf64_Phdr* phdr = NULL;
// .strtab
char* strTab = NULL;
// .shstrtab
char* shstrTab = NULL;
// .dynstr
char* dynStrTab = NULL;

int main() {

    FILE* fd = fopen("test", "r");
    // 读取文件大小
    struct stat fStatus;
    if(fstat(fileno(fd),&fStatus) == -1) {
        printf("Error fstat!!");
        exit(1);
    };
    int len = fStatus.st_size;
    if(len <= 0) {
        printf("Error get file len");
        exit(1);
    }
    char * buf = (char*) malloc(len);
    int readLen = fread(buf,1,len,fd);

    initGlobal(buf, readLen);
    printfElf();
    free(buf);

}

void initGlobal(char* buf, int len) {
    ehdr = (Elf64_Ehdr*) buf;
    shdr = (Elf64_Shdr*) (buf + ehdr->e_shoff);
    phdr = (Elf64_Phdr*) (buf + ehdr->e_phoff);
    // shstrTab
    Elf64_Shdr* strSectionHeader = shdr + ehdr->e_shstrndx;
    shstrTab = buf + strSectionHeader->sh_offset;

    // 记录.strtab section位置
    for(int i = 0;i < ehdr->e_shnum; i++) {
        // 遍历所有的section
        Elf64_Shdr* section = shdr + i;
        // check section 名称
        char * realName = shstrTab + section->sh_name;
        if(strcmp(realName, ".strtab") == 0) {
            strTab = buf + section->sh_offset; 
        } else if(strcmp(realName,".dynstr") == 0) {
            dynStrTab = buf + section->sh_offset;
        }
    }


}

void printfElf() {
    printfElfHeader(ehdr);
    printfSectionHeaders(ehdr);
    printfSection(ehdr,shdr);
}

void printfElfHeader(Elf64_Ehdr* header) {
    printf("\n这是Elf Header参数");

    printf("\ne_ident:\t");
    unsigned char *a = header->e_ident;
    for(int i = 0;i < 16; i++) {
        printf("%-02X ",a[i]);
    }
    printf("\ne_type:   \t%-02X",header->e_type);
    printf("\ne_machine:\t%-02X",header->e_machine);
    printf("\ne_version:\t%-04X",header->e_version);
    printf("\ne_entry:\t%-08X",header->e_entry);
    printf("\ne_phoff:\t%-08X",header->e_phoff);
    printf("\ne_shoff:\t%-08X",header->e_shoff);
    printf("\ne_flags:\t%-04X",header->e_flags);
    printf("\ne_ehsize:\t%-02X",header->e_ehsize);
    printf("\ne_phentsize:\t%-02X",header->e_phentsize);
    printf("\ne_phnum:\t%-02X",header->e_phnum);
    printf("\ne_shentsize:\t%-02X",header->e_shentsize);
    printf("\ne_shnum:\t%-02X",header->e_shnum);
    printf("\ne_shstrndx:\t%-02X",header->e_shstrndx);
}

void printfSectionHeaders(Elf64_Ehdr* header) {
    void* h = (void *) header;
    printf("\n\n如下是Elf Sections Header");
    Elf64_Off offset = header->e_shoff;
    int n = header->e_shnum;
    Elf64_Shdr* section = (Elf64_Shdr*)(h + offset);
    printf("\n%-03s %-018s %-08s %-016s %-016s %-08s %-016s %-016s %-08s %-016s %-016s"
    ,"idx","name","type","flag","execAddr","offset","size","link","info","align","entrySize");

    // 遍历Section Header Table，打印所有的Section
    for(int i = 0;i < n; i++) {
        Elf64_Shdr* shdr = section + i;
        printfSectionHeader(shdr,i);
    }
}


void printfSectionHeader(Elf64_Shdr* header,int index) {
    Elf64_Word name = header->sh_name;
    Elf64_Word type = header->sh_type;
    Elf64_Xword flag = header->sh_flags;
    Elf64_Addr execAddr = header->sh_addr;
    Elf64_Off fileOffset = header->sh_offset;
    Elf64_Xword size = header->sh_size;
    Elf64_Word link = header->sh_link;
    Elf64_Word info = header->sh_info;
    Elf64_Xword align = header->sh_addralign;
    Elf32_Xword entrySize = header->sh_entsize;
    printf("\n%03d %-018s %-8X %-16lX %-16lX %-8X %-16lX %-16lX %-8X %-16lX %-16lX",index,shstrTab + name,type,flag,execAddr,fileOffset,size,link,info,align,entrySize);
}


void printfSection(Elf64_Ehdr* ehdr, Elf64_Shdr* shdr) {

    printfSymTabSection(ehdr,shdr);
    printfRelaTabSection(ehdr,shdr);
    printfProgramHeader();
    printfDynamic();

}

void printfSymTabSection(Elf64_Ehdr* ehdr, Elf64_Shdr* shdr) {

    for(int i = 0;i < ehdr->e_shnum; i++) {
        Elf64_Shdr* section = shdr + i;
        // 从shstrtab section中读取section name
        char* sectionName = shstrTab + section->sh_name;
        // 计算section开始位置
        void* header = ehdr;
        void* sectionBegin = header + section->sh_offset;
        // 打印SYMTAB中的符号表
        if(section->sh_type == SHT_SYMTAB ||
           section->sh_type == SHT_DYNSYM ) {
            printf("\n\nSection %s :",sectionName);
            // 遍历打印所有符号
            int n = section->sh_size / section->sh_entsize;
            Elf64_Sym* symbol = sectionBegin;
            printf("\n%-3s %-38s %-8s %-8s %-8s %-16s %-16s","idx","name","info","other","shndx","value","size");
            char* sectionName = shstrTab + section->sh_name;
            for(int i = 0;i < n; i++) {
                Elf64_Sym* cur = symbol + i;
                //打印每一条entry内容
                Elf64_Word name = cur->st_name;
                unsigned char info = cur->st_info;
                unsigned char other = cur->st_other;
                Elf64_Section shndx = cur->st_shndx;
                Elf64_Addr value = cur->st_value;
                Elf64_Xword size = cur->st_size;

                char* realName = NULL;
                if(strcmp(sectionName,".symtab") == 0) {
                    realName = strTab + name;
                } else {
                    realName = dynStrTab + name;
                }
                printf("\n%-3d %-38s %-8x %-8x %-8x %-16lX %-16lX",i,realName,info,other,shndx,value,size);
            }
        }
    }

}

void printfRelaTabSection(Elf64_Ehdr* ehdr, Elf64_Shdr* shdr) {
    for(int i = 0;i < ehdr->e_shnum; i++) {
        Elf64_Shdr* sectionHeader = shdr + i;
        if(sectionHeader->sh_type == SHT_RELA) {
            char* sectionName = shstrTab + sectionHeader->sh_name;
            printf("\n\nSections: %s :",sectionName);
            Elf64_Rela* section = (void *)ehdr + sectionHeader->sh_offset;
            //打印内容
            printf("\n%-16s %-16s %-16s","offset","info","addend");
            int n = sectionHeader->sh_size / sectionHeader->sh_entsize;
            for(int i = 0;i < n; i++) {
                Elf64_Rela* rela = section + i;
                Elf64_Addr offset = rela->r_offset;
                Elf64_Xword info = rela->r_info;
                Elf64_Sxword addend = rela->r_addend;
                printf("\n%-16lx %-16lx %-16lx",offset,info,addend);
            }
            
        }
    }
}

void printfProgramHeader() {

    int n = ehdr->e_phnum;
    printf("\n\nProgram Headers:");
    printf("\n%-8s %-8s %-8s %-16s %-16s %-16s %-16s %-16s %-16s","idx","type","flag","offset","vaddr","paddr","filesz","memsz","align");
    for(int i = 0;i < n; i++) {
        Elf64_Phdr* ph = phdr + i;
        Elf64_Word type = ph->p_type;
        Elf64_Word flag = ph->p_flags;
        Elf64_Off offset = ph->p_offset;
        Elf64_Addr vaddr = ph->p_vaddr;
        Elf64_Addr paddr = ph->p_paddr;
        Elf64_Xword filesz = ph->p_filesz;
        Elf64_Xword memsz = ph->p_memsz;
        Elf64_Xword align = ph->p_align;
        printf("\n%-8d %-8x %-8x %-16lX %-16lX %-16lX %-16lX %-16lX %-16lX",i,type,flag,offset,vaddr,paddr,filesz,memsz,align);   
    }

}

void printfDynamic() {
    int n = ehdr->e_shnum;
    for(int i = 0;i < n;i++) {
        Elf64_Shdr* shi = shdr + i;
        if(strcmp(shstrTab + shi->sh_name,".dynamic") == 0) {
            printf("\n\nSections: %s :",shstrTab + shi->sh_name);
            printf("\n%-16s %-16s","tag","value");
            Elf64_Dyn* d =  (Elf64_Dyn*)((void* )ehdr + shi->sh_offset); 
            // DT_NULL为结尾标记，表示遍历完成
            while(d->d_tag != DT_NULL) {
                Elf64_Sxword tag = d->d_tag;
                Elf64_Xword value = d->d_un.d_val;
                if(tag == DT_NEEDED) {
                    printf("\n%-16s %-16s","DT_NEEDED",dynStrTab + value);
                } else if(tag == DT_PLTRELSZ) {
                    printf("\n%-16s %d bytes","DT_PLTRELSZ",value);
                } else if(tag == DT_PLTGOT) {
                    printf("\n%-16s 0x%-16x","DT_PLTGOT",value);
                } else if(tag == DT_HASH) {
                    printf("\n%-16s 0x%-16x","DT_HASH",value);
                } else if(tag == DT_STRTAB) {
                    printf("\n%-16s 0x%-16x","DT_STRTAB",value);
                } else if(tag == DT_SYMTAB) {
                    printf("\n%-16s 0x%-16x","DT_SYMTAB",value);
                } else if(tag == DT_RELA) {
                    printf("\n%-16s 0x%-16x","DT_RELA",value);
                } else if(tag == DT_RELASZ) {
                    printf("\n%-16s %d bytes","DT_RELASZ",value);
                } else if(tag == DT_RELAENT) {
                    printf("\n%-16s %d bytes","DT_RELAENT",value);
                } else if(tag == DT_STRSZ) {
                    printf("\n%-16s %d","DT_STRSZ",value);
                } else if(tag == DT_SYMENT) {
                    printf("\n%-16s %d","DT_SYMENT",value);
                } else if(tag == DT_INIT) {
                    printf("\n%-16s 0x%-16x","DT_INIT",value);
                } else if(tag == DT_FINI) {
                    printf("\n%-16s 0x%-16x","DT_FINI",value);
                } else if (tag == DT_SONAME) {
                    printf("\n%-16s %s","DT_SONAME",dynStrTab + value);
                } else if (tag == DT_RPATH) {
                    printf("\n%-16s %s","DT_RPATH",dynStrTab + value);
                } else if(tag == DT_PLTREL) {
                    printf("\n%-16s %x","DT_PLTREL",value);
                } else if(tag == DT_TEXTREL){
                    printf("\n%-16s 0x%-16x","DT_TEXTREL",value);
                } else if(tag == DT_JMPREL) {
                    printf("\n%-16s 0x%-16x","DT_JMPREL",value);
                } else if(tag == DT_INIT_ARRAY) {
                    printf("\n%-16s 0x%-16x","DT_INIT_ARRAY",value);
                } else if(tag == DT_FINI_ARRAY){
                    printf("\n%-16s 0x%-16x","DT_FINI_ARRAY",value);
                } else if(tag == DT_INIT_ARRAYSZ){
                    printf("\n%-16s %d","DT_INIT_ARRAYSZ",value);
                } else if(tag == DT_FINI_ARRAYSZ){
                    printf("\n%-16s %d","DT_FINI_ARRAYSZ",value);
                } else if(tag == DT_DEBUG) {
                    printf("\n%-16s 0x%x","DT_DEBUG",value);
                } else {
                    printf("\n%-16lx %-16lx",tag,value);
                }
                d++;
            }
        }
    }

}