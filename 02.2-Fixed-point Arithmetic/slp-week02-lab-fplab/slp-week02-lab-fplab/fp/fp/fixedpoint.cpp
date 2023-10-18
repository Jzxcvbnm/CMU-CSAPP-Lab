#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "fixedpoint.h"

// You can remove this once all of the functions are fully implemented
static Fixedpoint DUMMY;


Fixedpoint fixedpoint_create(uint64_t whole) {

  // TODO: implement
    Fixedpoint fp;
    fp.whole=whole;
    fp.frac=0;
    fp.flag=0;

    return fp;
}

//frac表示实际的小数大小，即0x8000000000000000表示0.5
Fixedpoint fixedpoint_create2(uint64_t whole, uint64_t frac) {
  // TODO: implement
    Fixedpoint fp;
    fp.whole=whole;
    fp.frac=frac;
    fp.flag=0;

    return fp;
}

//frac表示十六进制数移位后的小数大小，即"123.8"中的"8"为0x8000000000000000
Fixedpoint fixedpoint_create_from_hex(const char *hex) {
  // TODO: implement
    Fixedpoint fp;
    fp.whole=0;
    fp.frac=0;
    fp.flag=0;
    int i=0;
    int tflag=0;
    int whole_len=0;
    int frac_len=0;

    if(hex[0]=='-'){
        fp.flag=1;
        i++;
    }
    for(;hex[i]!='\0';i++){
        if(hex[i]=='.'){
            tflag=1;
            continue;
        }
        if(tflag==0){
            fp.whole=fp.whole<<4;
            if(hex[i]>='0'&&hex[i]<='9'){
                fp.whole+=hex[i]-'0';
            }
            else if(hex[i]>='a'&&hex[i]<='f'){
                fp.whole+=hex[i]-'a'+10;
            }
            else if(hex[i]>='A'&&hex[i]<='F'){
                fp.whole+=hex[i]-'A'+10;
            }
            else{
                fp.flag=2;
            }
            whole_len++;
        }
        else{
            fp.frac=fp.frac<<4;
            if(hex[i]>='0'&&hex[i]<='9'){
                fp.frac+=hex[i]-'0';
            }
            else if(hex[i]>='a'&&hex[i]<='f'){
                fp.frac+=hex[i]-'a'+10;
            }
            else if(hex[i]>='A'&&hex[i]<='F'){
                fp.frac+=hex[i]-'A'+10;
            }
            else{
                fp.flag=2;
            }
            frac_len++;
        }
    }

    //异常标注
    if(whole_len>16||frac_len>16){
        fp.flag=2;
    }
    //移位
    else{
        fp.frac=fp.frac<<(4*(16-frac_len));
    }

    return fp;
}

uint64_t fixedpoint_whole_part(Fixedpoint val) {

  // TODO: implement
    return val.whole;
}

uint64_t fixedpoint_frac_part(Fixedpoint val) {

  // TODO: implement
    return val.frac;
}

Fixedpoint fixedpoint_add(Fixedpoint left, Fixedpoint right) {

  // TODO: implement
    Fixedpoint fp;
    int lpreflag=left.flag;
    int rpreflag=right.flag;
    if(left.flag==1){
        left.whole=~left.whole;
        if(~left.frac>~left.frac+1){
            left.frac=~left.frac+1;
            left.whole++;
        }
        else{
            left.frac=~left.frac+1;
        }
    }
    if(right.flag==1){
        right.whole=~right.whole;
        if(~right.frac>~right.frac+1){
            right.frac=~right.frac+1;
            right.whole++;
        }
        else{
            right.frac=~right.frac+1;
        }
    }

    fp.whole=left.whole+right.whole;
    fp.frac=left.frac+right.frac;
    //判断是否进位，进位后的小数部分比原来小
    if((fp.frac<left.frac)||(fp.frac<right.frac)){
        fp.whole++;
    }

    //符号相同判断是否溢出
    if(left.flag==right.flag){
        if(left.flag==0){
            if((fp.whole>>63!=left.whole>>63)||(fp.whole>>63!=right.whole>>63)){
                fp.flag=4;
            }
            else{
                fp.flag=0;
            }
        }

        else{
            if((fp.whole>>63!=left.whole>>63)||(fp.whole>>63!=right.whole>>63)){
                fp.flag=7;
            }
            else{
                fp.flag=1;
            }
        }
    }

    //符号不同需判断正负
    else{
        if(left.flag==1){
            left.whole=~left.whole;
            if(~left.frac>~left.frac+1){
                left.frac=~left.frac+1;
                left.whole++;
            }
            else{
                left.frac=~left.frac+1;
            }

            left=fixedpoint_negate(left);

        }

        if(right.flag==1){
            right.whole=~right.whole;
            if(~right.frac>~right.frac+1){
                right.frac=~right.frac+1;
                right.whole++;
            }
            else{
                right.frac=~right.frac+1;
            }

            right=fixedpoint_negate(right);

        }

        //比较大小
        if(fixedpoint_compare(left, right)==1){
            fp.flag=lpreflag;
        }

        else if(fixedpoint_compare(left, right)==0){
            fp.flag=0;
        }

        else{
            fp.flag=rpreflag;
        }

    }

    //结果为负数则取反加一
    if(fp.flag==1||fp.flag==5||fp.flag==7){
        fp.whole=~fp.whole;
        if(~fp.frac>~fp.frac+1){
            fp.frac=~fp.frac+1;
            fp.whole++;
        }
        else{
            fp.frac=~fp.frac+1;
        }
    }

    return fp;
}

Fixedpoint fixedpoint_sub(Fixedpoint left, Fixedpoint right) {

  // TODO: implement
    Fixedpoint fp;

    //负数取反加一
    int lpreflag=left.flag;
    int rpreflag=right.flag;
    if(left.flag==1){
        left.whole=~left.whole;
        if(~left.frac>~left.frac+1){
            left.frac=~left.frac+1;
            left.whole++;
        }
        else{
            left.frac=~left.frac+1;
        }
    }
    if(right.flag==1){
        right.whole=~right.whole;
        if(~right.frac>~right.frac+1){
            right.frac=~right.frac+1;
            right.whole++;
        }
        else{
            right.frac=~right.frac+1;
        }
    }

    //被减数取反加一
    right.whole=~right.whole;
    if(~right.frac>~right.frac+1){
        right.frac=~right.frac+1;
        right.whole++;
    }

    else{
        right.frac=~right.frac+1;
    }

    right=fixedpoint_negate(right);

    fp.whole=left.whole+right.whole;
    fp.frac=left.frac+right.frac;

    //判断是否进位，进位后小数部分比原来小
    if((fp.frac<left.frac)||(fp.frac<right.frac)){
        fp.whole++;
    }

    //判断是否溢出
    if(lpreflag!=rpreflag){
        if(lpreflag==0){
            if((fp.whole>>63!=left.whole>>63)||(fp.whole>>63!=right.whole>>63)){
                fp.flag=4;
            }
            else{
                fp.flag=0;
            }
        }

        else{
            if((fp.whole>>63!=left.whole>>63)||(fp.whole>>63!=right.whole>>63)){
                fp.flag=7;
            }
            else{
                fp.flag=1;
            }
        }
    }

    //符号位一样需判断正负
    else{
        right.whole=~right.whole;
        if(~right.frac>~right.frac+1){
            right.frac=~right.frac+1;
            right.whole++;
        }

        else{
            right.frac=~right.frac+1;
        }

        right=fixedpoint_negate(right);

        if(fixedpoint_compare(left, right)==1){
            fp.flag=0;
        }

        else if(fixedpoint_compare(left, right)==0){
            fp.flag=0;
        }

        else{
            fp.flag=1;
        }

    }

    //结果为负数则取反加一
    if(fp.flag==1||fp.flag==5||fp.flag==7){
        fp.whole=~fp.whole;
        if(~fp.frac>~fp.frac+1){
            fp.frac=~fp.frac+1;
            fp.whole++;
        }
        else{
            fp.frac=~fp.frac+1;
        }
    }

    return fp;
}

//仅改变符号位
Fixedpoint fixedpoint_negate(Fixedpoint val) {

  // TODO: implement
    if(val.flag==0&&(!fixedpoint_is_zero(val))){
        val.flag=1;
    }

    else{
        val.flag=0;
    }
    return val;
}

//需判断下溢出
Fixedpoint fixedpoint_halve(Fixedpoint val) {
  // TODO: implement
    Fixedpoint fp;
    fp.flag=val.flag;
    if(val.flag==0){
        if(val.frac&0x0000000000000001UL==1){
            fp.flag=6;
        }

        if(val.whole&1==1){
            fp.frac=val.frac>>1;
            fp.whole=val.whole>>1;
            fp.frac+=0x8000000000000000UL;
        }
        else{
            fp.frac=val.frac>>1;
            fp.whole=val.whole>>1;
        }
    }
    else{
        //负数先取反再加一
        val.whole=~val.whole;
        if(~val.frac>~val.frac+1){
            val.frac=~val.frac+1;
            val.whole++;
        }
        else{
            val.frac=~val.frac+1;
        }

        if(val.frac&0x0000000000000001UL==1){
            fp.flag=5;
        }
        if(val.whole&1==1){
            fp.frac=val.frac>>1;
            fp.whole=val.whole>>1;
            fp.frac+=0x8000000000000000UL;
        }
        else{
            fp.frac=val.frac>>1;
            fp.whole=val.whole>>1;

        }
        //再取反加一
        fp.whole=~fp.whole;
        if(~fp.frac>~fp.frac+1){
            fp.frac=~fp.frac+1;
            fp.whole++;
        }
        else{
            fp.frac=~fp.frac+1;
        }
    }

    return fp;

}

//需判断上溢出
Fixedpoint fixedpoint_double(Fixedpoint val) {
  // TODO: implement
    Fixedpoint fp;
    fp.flag=val.flag;
    if(val.flag==0){
        if(val.whole&0x8000000000000000UL==1){
            fp.flag=4;
        }

        if((val.frac>>63)&1==1){
            fp.whole=val.whole<<1;
            fp.frac=val.frac<<1;
            fp.whole+=0x0000000000000001UL;
        }
        else{
            fp.whole=val.whole<<1;
            fp.frac=val.frac<<1;
        }
    }
    else{
        //负数先取反再加一
        val.whole=~val.whole;
        if(~val.frac>~val.frac+1){
            val.frac=~val.frac+1;
            val.whole++;
        }
        else{
            val.frac=~val.frac+1;
        }

        if(val.whole&0x8000000000000000UL==1){
            fp.flag=7;
        }

        if((val.frac>>63)&1==1){
            fp.whole=val.whole<<1;
            fp.frac=val.frac<<1;
            fp.whole+=0x0000000000000001UL;
        }
        else{
            fp.whole=val.whole<<1;
            fp.frac=val.frac<<1;
        }
        //再取反加一
        fp.whole=~fp.whole;
        if(~fp.frac>~fp.frac+1){
            fp.frac=~fp.frac+1;
            fp.whole++;
        }
        else{
            fp.frac=~fp.frac+1;
        }
    }

    return fp;

}

int fixedpoint_compare(Fixedpoint left, Fixedpoint right) {

  // TODO: implement
    if(left.flag==0&&right.flag==0){
        if(left.whole>right.whole){
            return 1;
        }
        else if(left.whole<right.whole){
            return -1;
        }
        else{
            if(left.frac>right.frac){
                return 1;
            }
            else if(left.frac<right.frac){
                return -1;
            }
            else{
                return 0;
            }
        }
    }
    else if(left.flag==0&&right.flag==1){
        return 1;
    }
    else if(left.flag==1&&right.flag==0){
        return -1;
    }
    else{
        if(left.whole>right.whole){
            return -1;
        }
        else if(left.whole<right.whole){
            return 1;
        }
        else{
            if(left.frac>right.frac){
                return -1;
            }
            else if(left.frac<right.frac){
                return 1;
            }
            else{
                return 0;
            }
        }
    }
}

int fixedpoint_is_zero(Fixedpoint val) {

  // TODO: implement
    int ret=(!val.whole)&&(!val.frac);
    return ret;
}

int fixedpoint_is_err(Fixedpoint val) {

  // TODO: implement
    if(val.flag==2){
        return 1;
    }
    else{
        return 0;
    }
}

int fixedpoint_is_neg(Fixedpoint val) {

  // TODO: implement
    if(val.flag==1){
        return 1;
    }
    else{
        return 0;
    }
}

int fixedpoint_is_overflow_neg(Fixedpoint val) {

  // TODO: implement
    if(val.flag==7){
        return 1;
    }
    else{
        return 0;
    }
}

int fixedpoint_is_overflow_pos(Fixedpoint val) {

  // TODO: implement
    if(val.flag==4){
        return 1;
    }
    else{
        return 0;
    }
}

int fixedpoint_is_underflow_neg(Fixedpoint val) {

  // TODO: implement
    if(val.flag==5){
        return 1;
    }
    else{
        return 0;
    }
}

int fixedpoint_is_underflow_pos(Fixedpoint val) {

  // TODO: implement
    if(val.flag==6){
        return 1;
    }
    else{
        return 0;
    }
}

int fixedpoint_is_valid(Fixedpoint val) {

  // TODO: implement
    if(val.flag==0||val.flag==1){
        return 1;
    }
    else{
        return 0;
    }
}

char *fixedpoint_format_as_hex(Fixedpoint val) {

  // TODO: implement
    char* str = (char*)calloc(35, sizeof(char));
    size_t i = 0;
    if (fixedpoint_is_err(val))
    {
        strcpy(str, "<invalid>");
    }

    else
    {
        if (fixedpoint_is_neg(val))
        {
            str[i++]= '-';
        }

        char whole[17];

        //转换整数部分为十六进制数然后存储到字符串中
        sprintf(whole, "%llx", val.whole);
        strcpy(str + i, whole);
        i += strlen(whole);

        //检查是否需要添加小数点
        if (val.frac == 0UL)
        {
            str[i] = '\0';
            return str;
        }
        else
        {
            str[i++] = '.';
        }

        //转换小数部分为十六进制数然后存储到字符串中
        char frac[17];
        // 去除多余的0
        int zeroes_removed = 0;
        uint64_t frac_copy = val.frac;
        while ((frac_copy & 0xFUL) == 0){
            frac_copy >>= 4;
            zeroes_removed++;
        }
        sprintf(frac, "%llx", frac_copy);

        //为数据补0，同create2
        for (int j = strlen(frac) + zeroes_removed; j < 16; j++){
            str[i++] = '0';
        }

        strcpy(str + i, frac);
    }

    return str;
}
