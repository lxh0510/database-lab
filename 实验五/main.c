#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"
typedef struct tuple
{
    int x;
    int y;
}tuple;

void Linear_Search(void)
{
    Buffer buf; /* A buffer */
    unsigned char *read_blk; /* A pointer to a block */
    unsigned char *disk_blk; /* A pointer to a block */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    /* Get a new block in the buffer */
    read_blk = getNewBlockInBuffer(&buf);
    disk_blk = getNewBlockInBuffer(&buf);
    int i = 0;
    int s_num = 17;
    int disk_num = 100;
    int count = 0;    //符合查询条件的元组数
    int blk_count = 0;  //写入的磁盘块数
    while(s_num>=17&&s_num<=48)
    {
        /* Read the block from the hard disk */
        if ((read_blk = readBlockFromDisk(s_num, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        /* Process the data in the block */
        int S_C = -1;
        int S_D = -1;
        int addr = -1;
        char str[5];
        printf("读入数据块 %d:\n",s_num);
        for (i = 0; i < 7; i++) //一个blk存7个元组加一个地址
        {

            for (int k = 0; k < 4; k++)
            {
                str[k] = *(read_blk + i*8 + k);
            }
            S_C = atoi(str);
            for (int k = 0; k < 4; k++)
            {
                str[k] = *(read_blk + i*8 + 4 + k);
            }
            S_D = atoi(str);
            if(S_C==128)
            {
                printf("(S.C=%d, S.D=%d)\n",S_C, S_D);
                memcpy(disk_blk+count*8,read_blk+i*8,8);    //将读取的内容写入待写入磁盘的块disk_blk
                count++;
                if(count==7) //写满一个块，将下一个块的地址写入块的末尾,并将该块写入磁盘
                {
                    itoa(disk_num+blk_count+1,disk_blk+count*8,10);
                    /* Write the block to the hard disk */
                    if (writeBlockToDisk(disk_blk, disk_num+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
                    {
                        perror("Writing Block Failed!\n");
                        return -1;
                    }
                    printf("注:结果写入磁盘:%d\n",disk_num+blk_count);
                    blk_count++;
                    count = 0;
                }
            }

        }
        for (int k = 0; k < 4; k++)
        {
            str[k] = *(read_blk + i*8 + k);
        }
        addr = atoi(str);
        freeBlockInBuffer(read_blk,&buf);
        s_num = addr;
    }
    if(count!=0)  // 遍历完毕后，将为填满的待写入块写入磁盘中
    {
        if (writeBlockToDisk(disk_blk, disk_num+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        printf("注:结果写入磁盘:%d\n",disk_num+blk_count);
        blk_count++;
    }
    printf("满足选择条件的元组一共%d个\n",(blk_count-1)*7+count);
    printf("IO读写一共%d次\n",buf.numIO);
    freeBuffer(&buf);
}

void block_sort(unsigned char *block)  //块内数据排序
{
    tuple data[7];
    char str[5];
    int i,j;
    for(i=0; i<7; i++)               // 读取数据
    {
        for (int k = 0; k < 4; k++)
        {
            str[k] = *(block + i*8 + k);
        }
        data[i].x = atoi(str);
        for (int k = 0; k < 4; k++)
        {
            str[k] = *(block + i*8 + 4 + k);
        }
        data[i].y = atoi(str);
    }
    for(i=0;i<6;i++)                    // 冒泡排序
    {
        for(j=0;j<6-i;j++)
        {
            if(data[j].x>data[j+1].x)
            {
                tuple t = data[j+1];
                data[j+1] = data[j];
                data[j] = t;
            }
        }
    }
    for(i=0;i<7;i++)                           // 将排好序的数据写回块中
    {
        itoa(data[i].x,str,10);
        for (int k = 0; k < 4; k++)
        {
            *(block + i*8 + k) = str[k];
        }
        itoa(data[i].y,str,10);
        for (int k = 0; k < 4; k++)
        {
            *(block + i*8 + 4 + k) = str[k];
        }
    }
}

//TPMMS算法,begin_blk,end_blk分别为排序磁盘块起始结束位置，write_blk为排序后写入磁盘起始位置
void TPMMS(int begin_blk,int end_blk ,int write_blk)
{
    Buffer buf; /* A buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    int group_num = 7;                    //缓冲区最多八个块，但我们需要一个块存放待写入磁盘数据，因此设定每组最多7个块
    int i;
    unsigned char *sort_blk[group_num];
    unsigned char *disk_blk;
    disk_blk = getNewBlockInBuffer(&buf);
    int blk_num = begin_blk;
    // 第一遍归并
    while(blk_num>=begin_blk&&blk_num<=end_blk)
    {
        int sort_num=((blk_num+group_num-1)<=end_blk)?group_num:(end_blk-blk_num+1);       // 确定每组个数
        for(i=0;i<sort_num;i++)        // 将每组数据块从磁盘读出
        {
            if ((sort_blk[i] = readBlockFromDisk(blk_num+i, &buf)) == NULL)
            {
                perror("Reading Block Failed!\n");
                return -1;
            }
            block_sort(sort_blk[i]);  // 将块内数据排序好
        }
        int blk_count = 0;                   // 每组排序过程中的块号
        int count = 0;                       // 块中排序过程中元组号
        int pointer[sort_num];               // 遍历过程每个块的指针
        char str[5];
        for(i=0;i<sort_num;i++)              // 每个块从头遍历
        {
            pointer[i] = 0;
        }
        int finished = 0;
        while(blk_count<sort_num&&!finished)
        {
            tuple data[sort_num];
            for(i=0;i<sort_num;i++)
            {
                if(pointer[i]<7)
                {
                    for (int k = 0; k < 4; k++)
                    {
                        str[k] = *(sort_blk[i] + pointer[i]*8 + k);
                    }
                    data[i].x =atoi(str);
                    for (int k = 0; k < 4; k++)
                    {
                        str[k] = *(sort_blk[i] + pointer[i]*8 + 4 + k);
                    }
                    data[i].y = atoi(str);
                }
                else data[i].x = 0x7fffffff;
            }
            int min = data[0].x;
            int min_pointer = 0;
            for(i=1;i<sort_num;i++)
            {
                if(min>data[i].x)
                {
                    min = data[i].x;
                    min_pointer = i;
                }
            }
            memcpy(disk_blk+count*8,sort_blk[min_pointer] + pointer[min_pointer]*8,8);
            count++;
            if(count==7)
            {
                itoa(blk_num+blk_count+1,disk_blk+count*8,10);
                if (writeBlockToDisk(disk_blk, blk_num+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
                {
                    perror("Writing Block Failed!\n");
                    return -1;
                }
                disk_blk = getNewBlockInBuffer(&buf);
                blk_count++;
                count = 0;
            }
            if(pointer[min_pointer]==7)     finished = 1;
            else pointer[min_pointer] = pointer[min_pointer]+1;
        }
        blk_num += sort_num;
        for(i=0;i<group_num;i++)
        {
            freeBlockInBuffer(sort_blk[i],&buf);
        }
    }
    freeBlockInBuffer(disk_blk,&buf);
    // 第二遍归并
    int group,last_group_num;
    if((end_blk-begin_blk+1)%group_num==0)                    //求组数和最后一组的块数
    {
        group = (end_blk-begin_blk+1)/group_num;
        last_group_num = group_num;
    }
    else
    {
        group = (end_blk-begin_blk+1)/group_num+1;
        last_group_num = (end_blk-begin_blk+1)%group_num;
    }
    int group_blk_num[group];                              // 求出每组块数
    for(i=0;i<group-1;i++)
    {
        group_blk_num[i] = group_num;
    }
    group_blk_num[group-1] = last_group_num;
    int group_pointer[group];                                 // 组内指针，表示正指向组内第几个块
    int blk_pointer[group];                                   // 块内指针，表示正指向块内第几个元组
    for(i=0;i<group;i++)
    {
        group_pointer[i] = 0;
        blk_pointer[i] = 0;
    }
    int blk_count = 0;                                            // 排序过程中写入磁盘块数
    int count = 0;                                                // 排序过程中块内元组数
    char str[5];
    for(i=0;i<group;i++)                                          //读入每个组内第一个块
    {
        if ((sort_blk[i] = readBlockFromDisk(begin_blk+group_num*i, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
    }
    disk_blk = getNewBlockInBuffer(&buf);
    while(blk_count<(end_blk-begin_blk+1))
    {
        tuple data[group];
        for(i=0;i<group;i++)                            // 读入每组对应元素
        {
            if(blk_pointer[i]<7)
            {
                for (int k = 0; k < 4; k++)
                {
                    str[k] = *(sort_blk[i] + blk_pointer[i]*8 + k);
                }
                data[i].x =atoi(str);
                for (int k = 0; k < 4; k++)
                {
                    str[k] = *(sort_blk[i] + blk_pointer[i]*8 + 4 + k);
                }
                data[i].y = atoi(str);
            }
            else data[i].x = 0x7fffffff;
        }
        int min = data[0].x;
        int min_pointer = 0;                            // 最小元素所在组数
        for(i=1; i<group; i++)
        {
            if(min>data[i].x)
            {
                min = data[i].x;
                min_pointer = i;
            }
        }
        if(min == 0x7fffffff)       break;
        printf("min:(%d,%d)\n",min,data[min_pointer].y);
        memcpy(disk_blk+count*8,sort_blk[min_pointer] + blk_pointer[min_pointer]*8,8);     //数据写入待写入块
        count++;
        if(count==7)
        {
            itoa(write_blk+blk_count+1,disk_blk+count*8,10);
            if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
            {
                perror("Writing Block Failed!\n");
                return -1;
            }
            printf("注:结果写入磁盘:%d\n",write_blk+blk_count);
            disk_blk = getNewBlockInBuffer(&buf);
            blk_count++;
            count = 0;
        }
        // 若读取到该块最后一个元组
        if(blk_pointer[min_pointer]==(group_num-1))
        {
            // 若该块也是组内最后一个块
            if(group_pointer[min_pointer]==group_blk_num[min_pointer]-1)
            {
                blk_pointer[min_pointer]++;
            }
            else
            {
                group_pointer[min_pointer]++;
                blk_pointer[min_pointer] = 0;
                freeBlockInBuffer(sort_blk[min_pointer],&buf);
                if ((sort_blk[min_pointer] = readBlockFromDisk(begin_blk+group_num*min_pointer+group_pointer[min_pointer], &buf)) == NULL)
                {
                    perror("Reading Block Failed!\n");
                    return -1;
                }
            }
        }
        else blk_pointer[min_pointer]++;
    }
    // 释放缓冲区
    for(i=0; i<group; i++)
    {
        freeBlockInBuffer(sort_blk[i],&buf);
    }
    freeBlockInBuffer(disk_blk,&buf);
    freeBuffer(&buf);
}

//创建索引,begin_blk,end_blk分别为建立索引磁盘块起始结束位置，write_blk为建立索引后写入磁盘起始位置
//索引形式为:(数据值，索引值)  索引值=块号*7+元组号
int createIndex(int begin_blk,int end_blk ,int write_blk)
{
    Buffer buf; /* A buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    unsigned char *read_blk;
    unsigned char *disk_blk;
    disk_blk = getNewBlockInBuffer(&buf);
    int read_blk_num;                               //读取磁盘块指针
    int write_blk_num = write_blk;                  //写入磁盘块指针
    int count = 0;                                  //写入到磁盘块中的第几个元组
    int index_data = 0;                             //索引对应数据值
    int index_num;                                  //属性值
    char str[5];
    // 遍历全部磁盘块，建立索引
    for(read_blk_num=begin_blk;read_blk_num<=end_blk;read_blk_num++)
    {
        if ((read_blk = readBlockFromDisk(read_blk_num, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        int data[7];                               //块中七个元组首个属性数据
        for(int i=0;i<7;i++)                           //读入数据
        {
            for (int k = 0; k < 4; k++)
            {
                str[k] = *(read_blk + i*8 + k);
            }
            data[i] =atoi(str);
            if(data[i]!=index_data)
            {
                index_data=data[i];
                index_num=read_blk_num*7+i;
                itoa(index_data,str,10);
                for (int k = 0; k < 4; k++)
                {
                    *(disk_blk + count*8 + k) = str[k];
                }
                itoa(index_num,str,10);
                for (int k = 0; k < 4; k++)
                {
                    *(disk_blk + count*8 + 4 + k) = str[k];
                }
                count++;
                if(count==7)                       //一个块写满写下一个块
                {
                    itoa(write_blk_num+1,disk_blk+count*8,10);
                    if (writeBlockToDisk(disk_blk, write_blk_num, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
                    {
                        perror("Writing Block Failed!\n");
                        return -1;
                    }
                    disk_blk = getNewBlockInBuffer(&buf);
                    write_blk_num++;
                    count = 0;
                }
            }
        }
        freeBlockInBuffer(read_blk,&buf);
    }
    if(count!=0)
    {
        itoa(write_blk_num+1,disk_blk+count*8,10);
        if (writeBlockToDisk(disk_blk, write_blk_num, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        disk_blk = getNewBlockInBuffer(&buf);
        write_blk_num++;
        count = 0;
    }
    freeBlockInBuffer(disk_blk,&buf);
    freeBuffer(&buf);
    return write_blk_num-write_blk;
}

void SelectByIndex(int index_blk,int blk_num,int number,int write_blk)
{
    Buffer buf; /* A buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    unsigned char *read_index_blk;
    unsigned char *read_data_blk;
    unsigned char *disk_blk;
    read_data_blk = getNewBlockInBuffer(&buf);
    disk_blk = getNewBlockInBuffer(&buf);
    int read_index_num = index_blk;                   //读取索引磁盘块指针
    int read_data_num = 0;                            //读取数据磁盘块指针
    int blk_count = 0;                                  //写入磁盘块数量
    int count = 0;                                  //写入到磁盘块中的第几个元组
    int tuple_num;
    char str[5];
    int finished = 0;
    int find = 0;
    int blk_last_data;               //索引块的首个数据和末个数据，用于定位待查找数据位置
    int start_index,end_index;
    tuple index[7];
    tuple data[7];
    while(!finished&&read_index_num<(index_blk+blk_num))
    {
        if ((read_index_blk = readBlockFromDisk(read_index_num, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        printf("读入索引块:%d\n",read_index_num);
        for (int k = 0; k < 4; k++)
        {
            str[k] = *(read_index_blk + 6*8 + k);
        }
        blk_last_data = atoi(str);
        if(blk_last_data<number)
        {
            printf("没有满足条件的元组.\n");
        }
        else
        {
            for(int i=0;i<7;i++)
            {
                for (int k = 0; k < 4; k++)
                {
                    str[k] = *(read_index_blk + i*8 + k);
                }
                index[i].x =atoi(str);
                for (int k = 0; k < 4; k++)
                {
                    str[k] = *(read_index_blk + i*8 + 4 + k);
                }
                index[i].y = atoi(str);
                if(index[i].x==number)
                {
                    start_index = index[i].y;
                    if(i==6)
                    {
                        if ((read_index_blk = readBlockFromDisk(read_index_num+1, &buf)) == NULL)
                        {
                            perror("Reading Block Failed!\n");
                            return -1;
                        }
                        for (int k = 0; k < 4; k++)
                        {
                            str[k] = *(read_index_blk  + 4 + k);
                        }
                        end_index = atoi(str);
                    }
                    else
                    {
                        for (int k = 0; k < 4; k++)
                        {
                            str[k] = *(read_index_blk + (i+1)*8 + 4 + k);
                        }
                        end_index = atoi(str);
                    }
                    for(int j=start_index;j<end_index;j++)
                    {
                        if((j/7)!=read_data_num)
                        {
                            freeBlockInBuffer(read_data_blk,&buf);
                            read_data_num = j/7;
                            if ((read_data_blk = readBlockFromDisk(read_data_num, &buf)) == NULL)
                            {
                                perror("Reading Block Failed!\n");
                                return -1;
                            }
                            printf("读入数据块%d\n",read_data_num);
                        }
                        tuple_num = j%7;
                        for (int k = 0; k < 4; k++)
                        {
                            str[k] = *(read_data_blk + tuple_num*8 + k);
                            *(disk_blk + count*8 + k) = str[k];
                        }
                        data[tuple_num].x =atoi(str);
                        for (int k = 0; k < 4; k++)
                        {
                            str[k] = *(read_data_blk + tuple_num*8 + 4 + k);
                            *(disk_blk + count*8 + 4 + k) = str[k];
                        }
                        data[tuple_num].y = atoi(str);
                        printf("(S.C=%d,S.D%d)\n",data[tuple_num].x,data[tuple_num].y);
                        count++;
                        if(count==7)
                        {
                            itoa(write_blk+blk_count+1,disk_blk+count*8,10);
                            if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
                            {
                                perror("Writing Block Failed!\n");
                                return -1;
                            }
                            printf("注:结果写入磁盘:%d\n",write_blk+blk_count);
                            disk_blk = getNewBlockInBuffer(&buf);
                            blk_count++;
                            count = 0;
                        }
                    }
                    finished = 1;
                    break;
                }
            }
        }
        read_index_num++;
        freeBlockInBuffer(read_index_blk,&buf);
    }
    if(count!=0)
    {
        itoa(write_blk+blk_count+1,disk_blk+count*8,10);
        if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        printf("注:结果写入磁盘:%d\n",write_blk+blk_count);
        disk_blk = getNewBlockInBuffer(&buf);
        blk_count++;
        count = 0;
    }
    freeBlockInBuffer(disk_blk,&buf);
    printf("满足选择条件的元组一共%d个\n",end_index-start_index);
    printf("IO读写一共%d次\n",buf.numIO);
    freeBuffer(&buf);
}

void Sort_Merge_Join(int s_begin_blk,int s_end_blk,int r_begin_blk,int r_end_blk,int write_blk)
{
    Buffer buf; /* A buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    int join_time;                                          //连接次数
    unsigned char *read_s_blk;                             //读取s数据磁盘块号
    unsigned char *read_r_blk;                             //读取r数据磁盘块号
    unsigned char *disk_blk;                               //待写入磁盘块
    if ((read_s_blk = readBlockFromDisk(s_begin_blk, &buf)) == NULL)
    {
        perror("Reading Block Failed!\n");
        return -1;
    }
    if ((read_r_blk = readBlockFromDisk(r_begin_blk, &buf)) == NULL)
    {
        perror("Reading Block Failed!\n");
        return -1;
    }
    disk_blk = getNewBlockInBuffer(&buf);
    char str[5];
    int s_count = 0;
    int r_count = 0;
    int blk_count = 0;
    int count = 0;
    int s_blk = s_begin_blk;
    int r_blk = r_begin_blk;
    tuple s_data,r_data;
    int s_pointer_data,r_pointer_data;                          //遍历s,r指针对应的数据
    memcpy(str,read_s_blk,4);                                   //s,r指针初始化为表中第一个数据
    s_pointer_data = atoi(str);
    memcpy(str,read_r_blk,4);
    r_pointer_data = atoi(str);
    int s_temp = s_pointer_data;
    int r_temp = r_pointer_data;
    int r_pointer_blk = r_blk , r_pointer_tuple = 0;           //r表遍历出新值时的指针，用于回溯
    int finished = 0;
    int isJoin = 0 , r_finished=0;                             //isJoin表示之前s表中元素是否发生连接，r_finished表示r表是否结束遍历
    //以s表为主指针,r表为副指针，遍历两表
    while(!finished)
    {
        //s表指针对应数值小于r表,s表指针前移
        if(s_pointer_data<r_pointer_data)
        {
            s_count++;
            //数据块读完，需要读下一个数据块
            if(s_count==7)
            {
                s_blk++;
                if(s_blk<=s_end_blk)
                {
                    freeBlockInBuffer(read_s_blk,&buf);
                    if ((read_s_blk = readBlockFromDisk(s_blk, &buf)) == NULL)
                    {
                        perror("Reading Block Failed!\n");
                        return -1;
                    }
                    s_count = 0;
                }
                else
                {
                    finished=1;
                }
            }
            if(!finished)
            {
                memcpy(str,read_s_blk+s_count*8,4);
                s_temp = atoi(str);
            }
            //若前移前s表数值进行了连接，且前移后s表数值不变,此时需要将r表回溯
            if(!finished&&isJoin&&s_temp==s_pointer_data)
            {
                if(r_blk!=r_pointer_blk)
                {
                    freeBlockInBuffer(read_r_blk,&buf);
                    if ((read_r_blk = readBlockFromDisk(r_pointer_blk, &buf)) == NULL)
                    {
                        perror("Reading Block Failed!\n");
                        return -1;
                    }
                }
                r_blk = r_pointer_blk;
                r_count = r_pointer_tuple;
                memcpy(str,read_r_blk+r_count*8,4);
                r_pointer_data = atoi(str);
            }
            //若不为上述情况，则更新指针对应数值
            else
            {
                isJoin = 0;
                if(r_finished)   finished = 1;
                if(s_temp!=s_pointer_data)
                {
                    s_pointer_data = s_temp;
                }
            }
        }
        //s表指针对应数值大于r表,r表指针前移,直至r表指针对应数值改变
        else if(s_pointer_data>r_pointer_data)
        {
            while(r_temp==r_pointer_data&&!finished)
            {
                r_count++;
                //数据块读完，需要读下一个数据块
                if(r_count==7)
                {
                    r_blk++;
                    if(r_blk<=r_end_blk)
                    {
                        freeBlockInBuffer(read_r_blk,&buf);
                        if ((read_r_blk = readBlockFromDisk(r_blk, &buf)) == NULL)
                        {
                            perror("Reading Block Failed!\n");
                            return -1;
                        }
                        r_count = 0;
                    }
                    else
                    {
                        finished=1;
                    }
                }
                if(!finished)
                {
                    memcpy(str,read_r_blk+r_count*8,4);
                    r_temp = atoi(str);
                }
            }
            //记录新值位置
            r_pointer_data = r_temp;
            r_pointer_blk = r_blk;
            r_pointer_tuple = r_count;
        }
        //s表指针对应数值等于r表，进行连接操作，并将r表指针前移
        else if(s_pointer_data==r_pointer_data)
        {
            join_time++;
            isJoin = 1;
            s_data.x = s_pointer_data;
            itoa(s_data.x,str,10);
            memcpy(disk_blk+count*16,str,4);
            memcpy(str,read_s_blk+s_count*8+4,4);
            memcpy(disk_blk+count*16+4,str,4);
            s_data.y = atoi(str);
            r_data.x = r_pointer_data;
            itoa(r_data.x,str,10);
            memcpy(disk_blk+count*16+8,str,4);
            memcpy(str,read_r_blk+r_count*8+4,4);
            memcpy(disk_blk+count*16+12,str,4);
            r_data.y = atoi(str);
            //printf("(%d,%d,%d,%d)\n",s_data.x,s_data.y,r_data.x,r_data.y);
            count++;
            //写满一个待写入块（找到三个连接数据），需要写入进磁盘
            if(count==3)
            {
                memset(disk_blk+3*16,0,8);                      //将待写入块第七个元组数据补齐
                itoa(write_blk+blk_count+1,disk_blk+7*8,10);
                if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
                {
                    perror("Writing Block Failed!\n");
                    return -1;
                }
                printf("注:结果写入磁盘:%d\n",write_blk+blk_count);
                disk_blk = getNewBlockInBuffer(&buf);
                blk_count++;
                count = 0;
            }
            r_count++;
            //数据块读完，需要读下一个数据块
            if(r_count==7)
            {
                r_blk++;
                if(r_blk<=r_end_blk)
                {
                    freeBlockInBuffer(read_r_blk,&buf);
                    if ((read_r_blk = readBlockFromDisk(r_blk, &buf)) == NULL)
                    {
                        perror("Reading Block Failed!\n");
                        return -1;
                    }
                    r_count = 0;
                }
                else
                {
                    r_finished = 1;
                    r_pointer_data+=1;
                }
            }
            if(!r_finished)
            {
                memcpy(str,read_r_blk+r_count*8,4);
                r_pointer_data = atoi(str);
            }
        }
    }
    //若最后剩余数据，写入一个新磁盘块
    if(count!=0)
    {
        itoa(write_blk+blk_count+1,disk_blk+7*8,10);
        if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        printf("注:结果写入磁盘:%d\n",write_blk+blk_count);
        disk_blk = getNewBlockInBuffer(&buf);
        blk_count++;
    }
    freeBlockInBuffer(disk_blk,&buf);
    freeBuffer(&buf);
    printf("总共连接%d次\n",join_time);
}

void intersection(int s_begin_blk,int s_end_blk,int r_begin_blk,int r_end_blk,int write_blk)
{
    Buffer buf; /* A buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    int intersection_num = 0;
    unsigned char *read_s_blk;                             //读取s数据磁盘块号
    unsigned char *read_r_blk;                             //读取r数据磁盘块号
    unsigned char *disk_blk;                               //待写入磁盘块
    if ((read_s_blk = readBlockFromDisk(s_begin_blk, &buf)) == NULL)
    {
        perror("Reading Block Failed!\n");
        return -1;
    }
    if ((read_r_blk = readBlockFromDisk(r_begin_blk, &buf)) == NULL)
    {
        perror("Reading Block Failed!\n");
        return -1;
    }
    disk_blk = getNewBlockInBuffer(&buf);
    char str[5];
    int s_count = 0;                                            //遍历到s表块中的元组序号
    int r_count = 0;                                            //遍历到r表块中的元组序号
    int blk_count = 0;
    int count = 0;
    int s_blk = s_begin_blk;                                    //遍历到s表的块号
    int r_blk = r_begin_blk;                                    //遍历到r表的块号
    int s_pointer_data,r_pointer_data;                          //遍历s,r指针对应的数据
    memcpy(str,read_s_blk,4);                                   //s,r指针初始化为表中第一个数据
    s_pointer_data = atoi(str);
    memcpy(str,read_r_blk,4);
    r_pointer_data = atoi(str);
    tuple s_data,r_data;
    int s_temp = s_pointer_data;
    int r_temp = r_pointer_data;
    int r_pointer_blk = r_blk , r_pointer_tuple = 0;             //r表遍历出新值时的指针，用于回溯
    int finished = 0,isJoin = 0,r_finished = 0;                  //isJoin表示之前s表中元素是否发生连接，r_finished表示r表是否结束遍历
    while(!finished)
    {
        if(s_pointer_data<r_pointer_data)
        {
            s_count++;
            //数据块读完，需要读下一个数据块
            if(s_count==7)
            {
                s_blk++;
                if(s_blk<=s_end_blk)
                {
                    freeBlockInBuffer(read_s_blk,&buf);
                    if ((read_s_blk = readBlockFromDisk(s_blk, &buf)) == NULL)
                    {
                        perror("Reading Block Failed!\n");
                        return -1;
                    }
                    s_count = 0;
                }
                else
                {
                    finished=1;
                }
            }
            if(!finished)
            {
                memcpy(str,read_s_blk+s_count*8,4);
                s_temp = atoi(str);
            }
            //若前移前s表数值进行了连接，且前移后s表数值不变,此时需要将r表回溯
            if(!finished&&isJoin&&s_temp==s_pointer_data)
            {
                if(r_blk!=r_pointer_blk)
                {
                    freeBlockInBuffer(read_r_blk,&buf);
                    if ((read_r_blk = readBlockFromDisk(r_pointer_blk, &buf)) == NULL)
                    {
                        perror("Reading Block Failed!\n");
                        return -1;
                    }
                }
                r_blk = r_pointer_blk;
                r_count = r_pointer_tuple;
                memcpy(str,read_r_blk+r_count*8,4);
                r_pointer_data = atoi(str);
            }
            //若不为上述情况，则更新指针对应数值
            else
            {
                isJoin = 0;
                if(r_finished)   finished = 1;
                if(s_temp!=s_pointer_data)
                {
                    s_pointer_data = s_temp;
                }
            }
        }
        else if(s_pointer_data>r_pointer_data)
        {
            while(r_temp==r_pointer_data&&!finished)
            {
                r_count++;
                //数据块读完，需要读下一个数据块
                if(r_count==7)
                {
                    r_blk++;
                    if(r_blk<=r_end_blk)
                    {
                        freeBlockInBuffer(read_r_blk,&buf);
                        if ((read_r_blk = readBlockFromDisk(r_blk, &buf)) == NULL)
                        {
                            perror("Reading Block Failed!\n");
                            return -1;
                        }
                        r_count = 0;
                    }
                    else
                    {
                        finished=1;
                    }
                }
                if(!finished)
                {
                    memcpy(str,read_r_blk+r_count*8,4);
                    r_temp = atoi(str);
                }
            }
            //r表指针对应数据更改，应记录更改后第一个数据位置便于回溯
            r_pointer_data = r_temp;
            r_pointer_blk = r_blk;
            r_pointer_tuple = r_count;
        }
        else if(s_pointer_data==r_pointer_data)
        {
            isJoin = 1;
            memcpy(str,read_s_blk+s_count*8+4,4);
            s_data.y = atoi(str);
            memcpy(str,read_r_blk+r_count*8+4,4);
            r_data.y = atoi(str);
            if(s_data.y==r_data.y)
            {
                intersection_num++;
                printf("(X=%d,Y=%d)\n",s_pointer_data,s_data.y);
                itoa(s_pointer_data,str,10);
                memcpy(disk_blk+count*8,str,4);
                itoa(s_data.y,str,10);
                memcpy(disk_blk+count*8+4,str,4);
                count++;
                if(count==7)
                {
                    itoa(write_blk+blk_count+1,disk_blk+count*8,10);
                    if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
                    {
                        perror("Writing Block Failed!\n");
                        return -1;
                    }
                    printf("注:结果写入磁盘:%d\n",write_blk+blk_count);
                    disk_blk = getNewBlockInBuffer(&buf);
                    blk_count++;
                    count = 0;
                }
            }
            r_count++;
            //数据块读完，需要读下一个数据块
            if(r_count==7)
            {
                r_blk++;
                if(r_blk<=r_end_blk)
                {
                    freeBlockInBuffer(read_r_blk,&buf);
                    if ((read_r_blk = readBlockFromDisk(r_blk, &buf)) == NULL)
                    {
                        perror("Reading Block Failed!\n");
                        return -1;
                    }
                    r_count = 0;
                }
                else
                {
                    r_finished = 1;
                    r_pointer_data+=1;
                }
            }
            if(!r_finished)
            {
                memcpy(str,read_r_blk+r_count*8,4);
                r_pointer_data = atoi(str);
            }
        }
    }
    if(count!=0)
    {
        itoa(write_blk+blk_count+1,disk_blk+7*8,10);
        if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        printf("注:结果写入磁盘:%d\n",write_blk+blk_count);
        disk_blk = getNewBlockInBuffer(&buf);
        blk_count++;
    }
    freeBlockInBuffer(disk_blk,&buf);
    freeBuffer(&buf);
    printf("S和R的交集共有%d个元组\n",intersection_num);
}

void UnionSet(int s_begin_blk,int s_end_blk,int r_begin_blk,int r_end_blk,int write_blk)
{
    Buffer buf; /* A buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    int union_num = 0;
    unsigned char *read_s_blk;                             //读取s数据磁盘块号
    unsigned char *read_r_blk;                             //读取r数据磁盘块号
    unsigned char *disk_blk;                               //待写入磁盘块
    if ((read_s_blk = readBlockFromDisk(s_begin_blk, &buf)) == NULL)
    {
        perror("Reading Block Failed!\n");
        return -1;
    }
    if ((read_r_blk = readBlockFromDisk(r_begin_blk, &buf)) == NULL)
    {
        perror("Reading Block Failed!\n");
        return -1;
    }
    disk_blk = getNewBlockInBuffer(&buf);
    char str[5];
    int s_count = 0;
    int r_count = 0;
    int blk_count = 0;
    int count = 0;
    int s_blk = s_begin_blk;
    int r_blk = r_begin_blk;
    int s_pointer_data,r_pointer_data;                          //遍历s,r指针对应的数据
    memcpy(str,read_s_blk,4);                                   //s,r指针初始化为表中第一个数据
    s_pointer_data = atoi(str);
    memcpy(str,read_r_blk,4);
    r_pointer_data = atoi(str);
    tuple s_data,r_data;
    int s_temp = s_pointer_data;
    int r_temp = r_pointer_data;
    int s_prev = s_pointer_data-1;                              //s表指针的前一个数据
    int r_prev = r_pointer_data-1;                              //r表指针的前一个数据
    int s_join = 0;                                             //s表是否存在和r表中某数据相同的数据
    int r_move = 0;                                             //r表指针是否需要改变
    int r_pointer_blk = r_blk , r_pointer_tuple = 0;
    int finished = 0,isJoin = 0,r_finished = 0;
    while(!finished)
    {
        if(s_pointer_data<r_pointer_data)
        {
            //printf("(X=%d)\n",s_pointer_data);
            if(!s_join)
            {
                itoa(s_pointer_data,str,10);
                memcpy(disk_blk+count*8,str,4);
                memcpy(str,read_s_blk+s_count*8+4,4);
                s_data.y = atoi(str);
                memcpy(disk_blk+count*8+4,str,4);
                //printf("(X=%d,Y=%d)\n",s_pointer_data,s_data.y);
                union_num++;
                count++;
                if(count==7)
                {
                    itoa(write_blk+blk_count+1,disk_blk+count*8,10);
                    if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
                    {
                        perror("Writing Block Failed!\n");
                        return -1;
                    }
                    printf("注:结果写入磁盘:%d\n",write_blk+blk_count);
                    disk_blk = getNewBlockInBuffer(&buf);
                    blk_count++;
                    count = 0;
                }
            }
            s_count++;
            s_join = 0;
            //数据块读完，需要读下一个数据块
            if(s_count==7)
            {
                s_blk++;
                if(s_blk<=s_end_blk)
                {
                    freeBlockInBuffer(read_s_blk,&buf);
                    if ((read_s_blk = readBlockFromDisk(s_blk, &buf)) == NULL)
                    {
                        perror("Reading Block Failed!\n");
                        return -1;
                    }
                    s_count = 0;
                }
                else
                {
                    finished=1;
                }
            }
            if(!finished)
            {
                memcpy(str,read_s_blk+s_count*8,4);
                s_temp = atoi(str);
                s_prev = s_pointer_data;
            }
            //若前移前s表数值进行了连接，且前移后s表数值不变,此时需要将r表回溯
            if(!finished&&isJoin&&s_temp==s_pointer_data)
            {
                if(r_blk!=r_pointer_blk)
                {
                    freeBlockInBuffer(read_r_blk,&buf);
                    if ((read_r_blk = readBlockFromDisk(r_pointer_blk, &buf)) == NULL)
                    {
                        perror("Reading Block Failed!\n");
                        return -1;
                    }
                }
                r_blk = r_pointer_blk;
                r_count = r_pointer_tuple;
                memcpy(str,read_r_blk+r_count*8,4);
                r_pointer_data = atoi(str);
            }
            //若不为上述情况，则更新指针对应数值
            else
            {
                isJoin = 0;
                if(s_temp!=s_pointer_data)
                {
                    s_pointer_data = s_temp;
                    r_move = 1;
                }
            }
        }
        else if(s_pointer_data>r_pointer_data)
        {
            //printf("(Y=%d)\n",r_pointer_data);
            itoa(r_pointer_data,str,10);
            memcpy(disk_blk+count*8,str,4);
            memcpy(str,read_r_blk+r_count*8+4,4);
            r_data.y = atoi(str);
            memcpy(disk_blk+count*8+4,str,4);
            //printf("(X=%d,Y=%d)\n",r_pointer_data,r_data.y);
            union_num++;
            count++;
            if(count==7)
            {
                itoa(write_blk+blk_count+1,disk_blk+count*8,10);
                if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
                {
                    perror("Writing Block Failed!\n");
                    return -1;
                }
                printf("注:结果写入磁盘:%d\n",write_blk+blk_count);
                disk_blk = getNewBlockInBuffer(&buf);
                blk_count++;
                count = 0;
            }
            r_count++;
            //数据块读完，需要读下一个数据块
            if(r_count==7)
            {
                r_blk++;
                if(r_blk<=r_end_blk)
                {
                    freeBlockInBuffer(read_r_blk,&buf);
                    if ((read_r_blk = readBlockFromDisk(r_blk, &buf)) == NULL)
                    {
                        perror("Reading Block Failed!\n");
                        return -1;
                    }
                    r_count = 0;
                }
                else
                {
                    finished=1;
                }
            }
            if(!finished)
            {
                memcpy(str,read_r_blk+r_count*8,4);
                r_temp = atoi(str);
                r_prev = r_pointer_data;
            }
            //更新r表指针
            if(r_temp!=r_pointer_data)
            {
                r_pointer_data = r_temp;
                r_pointer_blk = r_blk;
                r_pointer_tuple = r_count;
            }
        }
        else if(s_pointer_data==r_pointer_data)
        {
            //printf("(%d,%d)\n",s_prev,s_pointer_data);
            isJoin = 1;
            memcpy(str,read_s_blk+s_count*8+4,4);
            s_data.y = atoi(str);
            memcpy(str,read_r_blk+r_count*8+4,4);
            r_data.y = atoi(str);
            if(s_data.y==r_data.y&&s_prev==s_pointer_data)     s_join = 1;          //判断s表中数据是否与r表相同，避免重复读取
            if(s_data.y!=r_data.y&&s_prev!=s_pointer_data)
            {
                union_num++;
                //printf("(X=%d,Y=%d)\n",r_pointer_data,r_data.y);
                itoa(r_pointer_data,str,10);
                memcpy(disk_blk+count*8,str,4);
                itoa(r_data.y,str,10);
                memcpy(disk_blk+count*8+4,str,4);
                count++;
                if(count==7)
                {
                    itoa(write_blk+blk_count+1,disk_blk+count*8,10);
                    if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
                    {
                        perror("Writing Block Failed!\n");
                        return -1;
                    }
                    printf("注:结果写入磁盘:%d\n",write_blk+blk_count);
                    disk_blk = getNewBlockInBuffer(&buf);
                    blk_count++;
                    count = 0;
                }
            }
            if(r_prev!=r_pointer_data&&r_move)
            {
                r_pointer_blk = r_blk;
                r_pointer_tuple = r_count;
                r_move = 0;
            }
            r_count++;
            //数据块读完，需要读下一个数据块
            if(r_count==7)
            {
                r_blk++;
                if(r_blk<=r_end_blk)
                {
                    freeBlockInBuffer(read_r_blk,&buf);
                    if ((read_r_blk = readBlockFromDisk(r_blk, &buf)) == NULL)
                    {
                        perror("Reading Block Failed!\n");
                        return -1;
                    }
                    r_count = 0;
                }
                else
                {
                    r_finished = 1;
                    r_pointer_data = 9999;
                }
            }
            if(!r_finished)
            {
                memcpy(str,read_r_blk+r_count*8,4);
                r_temp = atoi(str);
                r_prev = r_pointer_data;
                r_pointer_data = r_temp;
            }
        }
    }

    if(count!=0)
    {
        itoa(write_blk+blk_count+1,disk_blk+7*8,10);
        if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        printf("注:结果写入磁盘:%d\n",write_blk+blk_count);
        disk_blk = getNewBlockInBuffer(&buf);
        blk_count++;
    }
    freeBlockInBuffer(disk_blk,&buf);
    freeBuffer(&buf);
    printf("S和R的并集共有%d个元组\n",union_num);
}

void DifferenceSet(int s_begin_blk,int s_end_blk,int r_begin_blk,int r_end_blk,int write_blk)
{
    Buffer buf; /* A buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    int difference_num = 0;
    unsigned char *read_s_blk;                             //读取s数据磁盘块号
    unsigned char *read_r_blk;                             //读取r数据磁盘块号
    unsigned char *disk_blk;                               //待写入磁盘块
    if ((read_s_blk = readBlockFromDisk(s_begin_blk, &buf)) == NULL)
    {
        perror("Reading Block Failed!\n");
        return -1;
    }
    if ((read_r_blk = readBlockFromDisk(r_begin_blk, &buf)) == NULL)
    {
        perror("Reading Block Failed!\n");
        return -1;
    }
    disk_blk = getNewBlockInBuffer(&buf);
    char str[5];
    int s_count = 0;
    int r_count = 0;
    int blk_count = 0;
    int count = 0;
    int s_blk = s_begin_blk;
    int r_blk = r_begin_blk;
    int s_pointer_data,r_pointer_data;                          //遍历s,r指针对应的数据
    memcpy(str,read_s_blk,4);                                   //s,r指针初始化为表中第一个数据
    s_pointer_data = atoi(str);
    memcpy(str,read_r_blk,4);
    r_pointer_data = atoi(str);
    tuple s_data,r_data;
    int s_temp = s_pointer_data;
    int r_temp = r_pointer_data;
    int s_prev = s_pointer_data-1;
    int r_prev = r_pointer_data-1;
    int s_join = 0;
    int r_move = 0;
    int r_pointer_blk = r_blk , r_pointer_tuple = 0;
    int finished = 0,isJoin = 0,r_finished = 0;
    while(!finished)
    {
        if(s_pointer_data<r_pointer_data)
        {
            if(!s_join)
            {
                itoa(s_pointer_data,str,10);
                memcpy(disk_blk+count*8,str,4);
                memcpy(str,read_s_blk+s_count*8+4,4);
                s_data.y = atoi(str);
                memcpy(disk_blk+count*8+4,str,4);
                //printf("(X=%d,Y=%d)\n",s_pointer_data,s_data.y);
                difference_num++;
                count++;
                if(count==7)
                {
                    itoa(write_blk+blk_count+1,disk_blk+count*8,10);
                    if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
                    {
                        perror("Writing Block Failed!\n");
                        return -1;
                    }
                    printf("注:结果写入磁盘:%d\n",write_blk+blk_count);
                    disk_blk = getNewBlockInBuffer(&buf);
                    blk_count++;
                    count = 0;
                }
            }
            s_count++;
            s_join = 0;
            //数据块读完，需要读下一个数据块
            if(s_count==7)
            {
                s_blk++;
                if(s_blk<=s_end_blk)
                {
                    freeBlockInBuffer(read_s_blk,&buf);
                    if ((read_s_blk = readBlockFromDisk(s_blk, &buf)) == NULL)
                    {
                        perror("Reading Block Failed!\n");
                        return -1;
                    }
                    s_count = 0;
                }
                else
                {
                    finished=1;
                }
            }
            if(!finished)
            {
                memcpy(str,read_s_blk+s_count*8,4);
                s_temp = atoi(str);
                s_prev = s_pointer_data;
            }
            //若前移前s表数值进行了连接，且前移后s表数值不变,此时需要将r表回溯
            if(!finished&&isJoin&&s_temp==s_pointer_data)
            {
                if(r_blk!=r_pointer_blk)
                {
                    freeBlockInBuffer(read_r_blk,&buf);
                    if ((read_r_blk = readBlockFromDisk(r_pointer_blk, &buf)) == NULL)
                    {
                        perror("Reading Block Failed!\n");
                        return -1;
                    }
                }
                r_blk = r_pointer_blk;
                r_count = r_pointer_tuple;
                memcpy(str,read_r_blk+r_count*8,4);
                r_pointer_data = atoi(str);
            }
            //若不为上述情况，则更新指针对应数值
            else
            {
                isJoin = 0;
                if(s_temp!=s_pointer_data)
                {
                    s_pointer_data = s_temp;
                    r_move = 1;
                }
            }
        }
        else if(s_pointer_data>r_pointer_data)
        {
            r_count++;
            //数据块读完，需要读下一个数据块
            if(r_count==7)
            {
                r_blk++;
                if(r_blk<=r_end_blk)
                {
                    freeBlockInBuffer(read_r_blk,&buf);
                    if ((read_r_blk = readBlockFromDisk(r_blk, &buf)) == NULL)
                    {
                        perror("Reading Block Failed!\n");
                        return -1;
                    }
                    r_count = 0;
                }
                else
                {
                    finished=1;
                }
            }
            if(!finished)
            {
                memcpy(str,read_r_blk+r_count*8,4);
                r_temp = atoi(str);
                r_prev = r_pointer_data;
            }
            if(r_temp!=r_pointer_data)
            {
                r_pointer_data = r_temp;
                r_pointer_blk = r_blk;
                r_pointer_tuple = r_count;
            }
        }
        else if(s_pointer_data==r_pointer_data)
        {
            isJoin = 1;
            memcpy(str,read_s_blk+s_count*8+4,4);
            s_data.y = atoi(str);
            memcpy(str,read_r_blk+r_count*8+4,4);
            r_data.y = atoi(str);
            if(s_data.y==r_data.y)     s_join = 1;
            if(r_prev!=r_pointer_data&&r_move)
            {
                r_pointer_blk = r_blk;
                r_pointer_tuple = r_count;
                r_move = 0;
            }
            r_count++;
            //数据块读完，需要读下一个数据块
            if(r_count==7)
            {
                r_blk++;
                if(r_blk<=r_end_blk)
                {
                    freeBlockInBuffer(read_r_blk,&buf);
                    if ((read_r_blk = readBlockFromDisk(r_blk, &buf)) == NULL)
                    {
                        perror("Reading Block Failed!\n");
                        return -1;
                    }
                    r_count = 0;
                }
                else
                {
                    r_finished = 1;
                    r_pointer_data = 9999;
                }
            }
            if(!r_finished)
            {
                memcpy(str,read_r_blk+r_count*8,4);
                r_temp = atoi(str);
                r_prev = r_pointer_data;
                r_pointer_data = r_temp;
            }
        }
    }
    if(count!=0)
    {
        itoa(write_blk+blk_count+1,disk_blk+7*8,10);
        if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk函数中含有free内存块的操作，因此不需再使用freeBlockInBuffer函数
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        printf("注:结果写入磁盘:%d\n",write_blk+blk_count);
        disk_blk = getNewBlockInBuffer(&buf);
        blk_count++;
    }
    freeBlockInBuffer(disk_blk,&buf);
    freeBuffer(&buf);
    printf("S和R的差集共有%d个元组\n",difference_num);
}
void task1(void)
{
    printf("=================================\n");
    printf("基于线性搜索的选择算法S.C=128\n");
    printf("=================================\n");
    Linear_Search();
}

void task2(void)
{
    printf("=================================\n");
    printf("两阶段多路归并排序算法(TPMMS)\n");
    printf("=================================\n");
    TPMMS(1,16,301);
    TPMMS(17,48,317);
}

void task3(void)
{
    printf("=================================\n");
    printf("基于索引的关系选择算法S.C=128\n");
    printf("=================================\n");
    int index_blk_num = createIndex(317,348,401);
    SelectByIndex(401,index_blk_num,128,450);
}

void task4(void)
{
    printf("=================================\n");
    printf("基于排序的连接算法\n");
    printf("=================================\n");
    Sort_Merge_Join(317,348,301,316,500);
}

void task5_1(void)
{
    printf("=================================\n");
    printf("基于排序的集合的交运算\n");
    printf("=================================\n");
    intersection(317,348,301,316,650);
}

void task5_2(void)
{
    printf("=================================\n");
    printf("基于排序的集合的并运算\n");
    printf("=================================\n");
    UnionSet(317,348,301,316,700);
}

void task5_3(void)
{
    printf("=================================\n");
    printf("基于排序的集合的差运算\n");
    printf("=================================\n");
    DifferenceSet(317,348,301,316,750);
}
int main()
{
     //task1();
    //task2();
    //task3();
    //task4();
    //task5_1();
    //task5_2();
    task5_3();
    return 0;
}
