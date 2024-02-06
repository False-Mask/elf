#include <elf.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <elf.h>

#include <string.h>

// 打印整个ELF文件
void printfElf(void* buf,int len);
// 打印ELF Header
void printfElfHeader(Elf64_Ehdr* header);
// 打印ELF所有Section Headers
void printfSectionHeaders(Elf64_Ehdr* header);
// 打印ELF指定的Section Header
void printfSectionHeader(Elf64_Shdr* header,int index);
// 打印Section内容
void printfSection(Elf64_Ehdr*,Elf64_Shdr*);

// .strtab
char* strTab = NULL;
// .shstrtab
char* shstrTab = NULL;

// 打印.symtab section
void printfSymTabSection(Elf64_Sym* symbol,int n);

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

    printfElf(buf,readLen);

    free(buf);


}

void printfElf(void *buf,int len) {
    Elf64_Ehdr * elf = (Elf64_Ehdr*) buf;
    printfElfHeader(elf);
    printfSectionHeaders(elf);

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

    // 获取string table
    Elf64_Shdr* strSectionHeader = section + header->e_shstrndx;
    char* strSection = h + strSectionHeader->sh_offset;
    shstrTab = h + strSectionHeader->sh_offset;
    // 遍历Section Header Table，打印所有的Section
    for(int i = 0;i < n; i++) {
        Elf64_Shdr* shdr = section + i;
        printfSectionHeader(shdr,i);
        // 记录.strtab section位置
        if(strcmp(strSection + shdr->sh_name, ".strtab") == 0) {
            strTab = h + shdr->sh_offset; 
        }
    }

    // check string table是否寻找到
    if(strTab == NULL) {
        printf("\nerror: 无法找到.strtab section");
    }

    for(int i = 0;i < n; i++) {
        printfSection(header,section + i);
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
    printf("\n%03d %-018s %-8X %-16X %-16X %-8X %-16X %-16X %-8X %-16X %-16X",index,shstrTab + name,type,flag,execAddr,fileOffset,size,link,info,align,entrySize);
}


void printfSection(Elf64_Ehdr* ehdr, Elf64_Shdr* shdr) {
    // 从shstrtab section中读取section name
    char* sectionName = shstrTab + shdr->sh_name;
    // 计算section开始位置
    void* header = ehdr;
    void* sectionBegin = header + shdr->sh_offset;
    // 打印SYMTAB中的符号表
    if(shdr->sh_type == SHT_SYMTAB) {
        printf("\n\nSection %s :",sectionName);
        printfSymTabSection(sectionBegin,shdr->sh_size / shdr->sh_entsize);
    }

}

void printfSymTabSection(Elf64_Sym* symbol,int n) {
    printf("\n%-3s %-38s %-8s %-8s %-8s %-16s %-16s","idx","name","info","other","shndx","value","size");
    for(int i = 0;i < n; i++) {
        Elf64_Sym* cur = symbol + i;
        //打印每一条entry内容
        Elf64_Word name = cur->st_name;
        unsigned char info = cur->st_info;
        unsigned char other = cur->st_other;
        Elf64_Section shndx = cur->st_shndx;
        Elf64_Addr value = cur->st_value;
        Elf64_Xword size = cur->st_size;
        printf("\n%-3d %-38s %-8x %-8x %-8x %-16x %-16x",i,strTab + name,info,other,shndx,value,size);
    }
}
