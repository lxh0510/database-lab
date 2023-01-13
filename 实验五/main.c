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
    int count = 0;    //���ϲ�ѯ������Ԫ����
    int blk_count = 0;  //д��Ĵ��̿���
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
        printf("�������ݿ� %d:\n",s_num);
        for (i = 0; i < 7; i++) //һ��blk��7��Ԫ���һ����ַ
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
                memcpy(disk_blk+count*8,read_blk+i*8,8);    //����ȡ������д���д����̵Ŀ�disk_blk
                count++;
                if(count==7) //д��һ���飬����һ����ĵ�ַд����ĩβ,�����ÿ�д�����
                {
                    itoa(disk_num+blk_count+1,disk_blk+count*8,10);
                    /* Write the block to the hard disk */
                    if (writeBlockToDisk(disk_blk, disk_num+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
                    {
                        perror("Writing Block Failed!\n");
                        return -1;
                    }
                    printf("ע:���д�����:%d\n",disk_num+blk_count);
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
    if(count!=0)  // ������Ϻ󣬽�Ϊ�����Ĵ�д���д�������
    {
        if (writeBlockToDisk(disk_blk, disk_num+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        printf("ע:���д�����:%d\n",disk_num+blk_count);
        blk_count++;
    }
    printf("����ѡ��������Ԫ��һ��%d��\n",(blk_count-1)*7+count);
    printf("IO��дһ��%d��\n",buf.numIO);
    freeBuffer(&buf);
}

void block_sort(unsigned char *block)  //������������
{
    tuple data[7];
    char str[5];
    int i,j;
    for(i=0; i<7; i++)               // ��ȡ����
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
    for(i=0;i<6;i++)                    // ð������
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
    for(i=0;i<7;i++)                           // ���ź��������д�ؿ���
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

//TPMMS�㷨,begin_blk,end_blk�ֱ�Ϊ������̿���ʼ����λ�ã�write_blkΪ�����д�������ʼλ��
void TPMMS(int begin_blk,int end_blk ,int write_blk)
{
    Buffer buf; /* A buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    int group_num = 7;                    //���������˸��飬��������Ҫһ�����Ŵ�д��������ݣ�����趨ÿ�����7����
    int i;
    unsigned char *sort_blk[group_num];
    unsigned char *disk_blk;
    disk_blk = getNewBlockInBuffer(&buf);
    int blk_num = begin_blk;
    // ��һ��鲢
    while(blk_num>=begin_blk&&blk_num<=end_blk)
    {
        int sort_num=((blk_num+group_num-1)<=end_blk)?group_num:(end_blk-blk_num+1);       // ȷ��ÿ�����
        for(i=0;i<sort_num;i++)        // ��ÿ�����ݿ�Ӵ��̶���
        {
            if ((sort_blk[i] = readBlockFromDisk(blk_num+i, &buf)) == NULL)
            {
                perror("Reading Block Failed!\n");
                return -1;
            }
            block_sort(sort_blk[i]);  // ���������������
        }
        int blk_count = 0;                   // ÿ����������еĿ��
        int count = 0;                       // �������������Ԫ���
        int pointer[sort_num];               // ��������ÿ�����ָ��
        char str[5];
        for(i=0;i<sort_num;i++)              // ÿ�����ͷ����
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
                if (writeBlockToDisk(disk_blk, blk_num+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
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
    // �ڶ���鲢
    int group,last_group_num;
    if((end_blk-begin_blk+1)%group_num==0)                    //�����������һ��Ŀ���
    {
        group = (end_blk-begin_blk+1)/group_num;
        last_group_num = group_num;
    }
    else
    {
        group = (end_blk-begin_blk+1)/group_num+1;
        last_group_num = (end_blk-begin_blk+1)%group_num;
    }
    int group_blk_num[group];                              // ���ÿ�����
    for(i=0;i<group-1;i++)
    {
        group_blk_num[i] = group_num;
    }
    group_blk_num[group-1] = last_group_num;
    int group_pointer[group];                                 // ����ָ�룬��ʾ��ָ�����ڵڼ�����
    int blk_pointer[group];                                   // ����ָ�룬��ʾ��ָ����ڵڼ���Ԫ��
    for(i=0;i<group;i++)
    {
        group_pointer[i] = 0;
        blk_pointer[i] = 0;
    }
    int blk_count = 0;                                            // ���������д����̿���
    int count = 0;                                                // ��������п���Ԫ����
    char str[5];
    for(i=0;i<group;i++)                                          //����ÿ�����ڵ�һ����
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
        for(i=0;i<group;i++)                            // ����ÿ���ӦԪ��
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
        int min_pointer = 0;                            // ��СԪ����������
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
        memcpy(disk_blk+count*8,sort_blk[min_pointer] + blk_pointer[min_pointer]*8,8);     //����д���д���
        count++;
        if(count==7)
        {
            itoa(write_blk+blk_count+1,disk_blk+count*8,10);
            if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
            {
                perror("Writing Block Failed!\n");
                return -1;
            }
            printf("ע:���д�����:%d\n",write_blk+blk_count);
            disk_blk = getNewBlockInBuffer(&buf);
            blk_count++;
            count = 0;
        }
        // ����ȡ���ÿ����һ��Ԫ��
        if(blk_pointer[min_pointer]==(group_num-1))
        {
            // ���ÿ�Ҳ���������һ����
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
    // �ͷŻ�����
    for(i=0; i<group; i++)
    {
        freeBlockInBuffer(sort_blk[i],&buf);
    }
    freeBlockInBuffer(disk_blk,&buf);
    freeBuffer(&buf);
}

//��������,begin_blk,end_blk�ֱ�Ϊ�����������̿���ʼ����λ�ã�write_blkΪ����������д�������ʼλ��
//������ʽΪ:(����ֵ������ֵ)  ����ֵ=���*7+Ԫ���
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
    int read_blk_num;                               //��ȡ���̿�ָ��
    int write_blk_num = write_blk;                  //д����̿�ָ��
    int count = 0;                                  //д�뵽���̿��еĵڼ���Ԫ��
    int index_data = 0;                             //������Ӧ����ֵ
    int index_num;                                  //����ֵ
    char str[5];
    // ����ȫ�����̿飬��������
    for(read_blk_num=begin_blk;read_blk_num<=end_blk;read_blk_num++)
    {
        if ((read_blk = readBlockFromDisk(read_blk_num, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        int data[7];                               //�����߸�Ԫ���׸���������
        for(int i=0;i<7;i++)                           //��������
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
                if(count==7)                       //һ����д��д��һ����
                {
                    itoa(write_blk_num+1,disk_blk+count*8,10);
                    if (writeBlockToDisk(disk_blk, write_blk_num, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
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
        if (writeBlockToDisk(disk_blk, write_blk_num, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
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
    int read_index_num = index_blk;                   //��ȡ�������̿�ָ��
    int read_data_num = 0;                            //��ȡ���ݴ��̿�ָ��
    int blk_count = 0;                                  //д����̿�����
    int count = 0;                                  //д�뵽���̿��еĵڼ���Ԫ��
    int tuple_num;
    char str[5];
    int finished = 0;
    int find = 0;
    int blk_last_data;               //��������׸����ݺ�ĩ�����ݣ����ڶ�λ����������λ��
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
        printf("����������:%d\n",read_index_num);
        for (int k = 0; k < 4; k++)
        {
            str[k] = *(read_index_blk + 6*8 + k);
        }
        blk_last_data = atoi(str);
        if(blk_last_data<number)
        {
            printf("û������������Ԫ��.\n");
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
                            printf("�������ݿ�%d\n",read_data_num);
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
                            if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
                            {
                                perror("Writing Block Failed!\n");
                                return -1;
                            }
                            printf("ע:���д�����:%d\n",write_blk+blk_count);
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
        if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        printf("ע:���д�����:%d\n",write_blk+blk_count);
        disk_blk = getNewBlockInBuffer(&buf);
        blk_count++;
        count = 0;
    }
    freeBlockInBuffer(disk_blk,&buf);
    printf("����ѡ��������Ԫ��һ��%d��\n",end_index-start_index);
    printf("IO��дһ��%d��\n",buf.numIO);
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
    int join_time;                                          //���Ӵ���
    unsigned char *read_s_blk;                             //��ȡs���ݴ��̿��
    unsigned char *read_r_blk;                             //��ȡr���ݴ��̿��
    unsigned char *disk_blk;                               //��д����̿�
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
    int s_pointer_data,r_pointer_data;                          //����s,rָ���Ӧ������
    memcpy(str,read_s_blk,4);                                   //s,rָ���ʼ��Ϊ���е�һ������
    s_pointer_data = atoi(str);
    memcpy(str,read_r_blk,4);
    r_pointer_data = atoi(str);
    int s_temp = s_pointer_data;
    int r_temp = r_pointer_data;
    int r_pointer_blk = r_blk , r_pointer_tuple = 0;           //r���������ֵʱ��ָ�룬���ڻ���
    int finished = 0;
    int isJoin = 0 , r_finished=0;                             //isJoin��ʾ֮ǰs����Ԫ���Ƿ������ӣ�r_finished��ʾr���Ƿ��������
    //��s��Ϊ��ָ��,r��Ϊ��ָ�룬��������
    while(!finished)
    {
        //s��ָ���Ӧ��ֵС��r��,s��ָ��ǰ��
        if(s_pointer_data<r_pointer_data)
        {
            s_count++;
            //���ݿ���꣬��Ҫ����һ�����ݿ�
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
            //��ǰ��ǰs����ֵ���������ӣ���ǰ�ƺ�s����ֵ����,��ʱ��Ҫ��r�����
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
            //����Ϊ��������������ָ���Ӧ��ֵ
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
        //s��ָ���Ӧ��ֵ����r��,r��ָ��ǰ��,ֱ��r��ָ���Ӧ��ֵ�ı�
        else if(s_pointer_data>r_pointer_data)
        {
            while(r_temp==r_pointer_data&&!finished)
            {
                r_count++;
                //���ݿ���꣬��Ҫ����һ�����ݿ�
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
            //��¼��ֵλ��
            r_pointer_data = r_temp;
            r_pointer_blk = r_blk;
            r_pointer_tuple = r_count;
        }
        //s��ָ���Ӧ��ֵ����r���������Ӳ���������r��ָ��ǰ��
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
            //д��һ����д��飨�ҵ������������ݣ�����Ҫд�������
            if(count==3)
            {
                memset(disk_blk+3*16,0,8);                      //����д�����߸�Ԫ�����ݲ���
                itoa(write_blk+blk_count+1,disk_blk+7*8,10);
                if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
                {
                    perror("Writing Block Failed!\n");
                    return -1;
                }
                printf("ע:���д�����:%d\n",write_blk+blk_count);
                disk_blk = getNewBlockInBuffer(&buf);
                blk_count++;
                count = 0;
            }
            r_count++;
            //���ݿ���꣬��Ҫ����һ�����ݿ�
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
    //�����ʣ�����ݣ�д��һ���´��̿�
    if(count!=0)
    {
        itoa(write_blk+blk_count+1,disk_blk+7*8,10);
        if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        printf("ע:���д�����:%d\n",write_blk+blk_count);
        disk_blk = getNewBlockInBuffer(&buf);
        blk_count++;
    }
    freeBlockInBuffer(disk_blk,&buf);
    freeBuffer(&buf);
    printf("�ܹ�����%d��\n",join_time);
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
    unsigned char *read_s_blk;                             //��ȡs���ݴ��̿��
    unsigned char *read_r_blk;                             //��ȡr���ݴ��̿��
    unsigned char *disk_blk;                               //��д����̿�
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
    int s_count = 0;                                            //������s����е�Ԫ�����
    int r_count = 0;                                            //������r����е�Ԫ�����
    int blk_count = 0;
    int count = 0;
    int s_blk = s_begin_blk;                                    //������s��Ŀ��
    int r_blk = r_begin_blk;                                    //������r��Ŀ��
    int s_pointer_data,r_pointer_data;                          //����s,rָ���Ӧ������
    memcpy(str,read_s_blk,4);                                   //s,rָ���ʼ��Ϊ���е�һ������
    s_pointer_data = atoi(str);
    memcpy(str,read_r_blk,4);
    r_pointer_data = atoi(str);
    tuple s_data,r_data;
    int s_temp = s_pointer_data;
    int r_temp = r_pointer_data;
    int r_pointer_blk = r_blk , r_pointer_tuple = 0;             //r���������ֵʱ��ָ�룬���ڻ���
    int finished = 0,isJoin = 0,r_finished = 0;                  //isJoin��ʾ֮ǰs����Ԫ���Ƿ������ӣ�r_finished��ʾr���Ƿ��������
    while(!finished)
    {
        if(s_pointer_data<r_pointer_data)
        {
            s_count++;
            //���ݿ���꣬��Ҫ����һ�����ݿ�
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
            //��ǰ��ǰs����ֵ���������ӣ���ǰ�ƺ�s����ֵ����,��ʱ��Ҫ��r�����
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
            //����Ϊ��������������ָ���Ӧ��ֵ
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
                //���ݿ���꣬��Ҫ����һ�����ݿ�
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
            //r��ָ���Ӧ���ݸ��ģ�Ӧ��¼���ĺ��һ������λ�ñ��ڻ���
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
                    if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
                    {
                        perror("Writing Block Failed!\n");
                        return -1;
                    }
                    printf("ע:���д�����:%d\n",write_blk+blk_count);
                    disk_blk = getNewBlockInBuffer(&buf);
                    blk_count++;
                    count = 0;
                }
            }
            r_count++;
            //���ݿ���꣬��Ҫ����һ�����ݿ�
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
        if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        printf("ע:���д�����:%d\n",write_blk+blk_count);
        disk_blk = getNewBlockInBuffer(&buf);
        blk_count++;
    }
    freeBlockInBuffer(disk_blk,&buf);
    freeBuffer(&buf);
    printf("S��R�Ľ�������%d��Ԫ��\n",intersection_num);
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
    unsigned char *read_s_blk;                             //��ȡs���ݴ��̿��
    unsigned char *read_r_blk;                             //��ȡr���ݴ��̿��
    unsigned char *disk_blk;                               //��д����̿�
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
    int s_pointer_data,r_pointer_data;                          //����s,rָ���Ӧ������
    memcpy(str,read_s_blk,4);                                   //s,rָ���ʼ��Ϊ���е�һ������
    s_pointer_data = atoi(str);
    memcpy(str,read_r_blk,4);
    r_pointer_data = atoi(str);
    tuple s_data,r_data;
    int s_temp = s_pointer_data;
    int r_temp = r_pointer_data;
    int s_prev = s_pointer_data-1;                              //s��ָ���ǰһ������
    int r_prev = r_pointer_data-1;                              //r��ָ���ǰһ������
    int s_join = 0;                                             //s���Ƿ���ں�r����ĳ������ͬ������
    int r_move = 0;                                             //r��ָ���Ƿ���Ҫ�ı�
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
                    if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
                    {
                        perror("Writing Block Failed!\n");
                        return -1;
                    }
                    printf("ע:���д�����:%d\n",write_blk+blk_count);
                    disk_blk = getNewBlockInBuffer(&buf);
                    blk_count++;
                    count = 0;
                }
            }
            s_count++;
            s_join = 0;
            //���ݿ���꣬��Ҫ����һ�����ݿ�
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
            //��ǰ��ǰs����ֵ���������ӣ���ǰ�ƺ�s����ֵ����,��ʱ��Ҫ��r�����
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
            //����Ϊ��������������ָ���Ӧ��ֵ
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
                if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
                {
                    perror("Writing Block Failed!\n");
                    return -1;
                }
                printf("ע:���д�����:%d\n",write_blk+blk_count);
                disk_blk = getNewBlockInBuffer(&buf);
                blk_count++;
                count = 0;
            }
            r_count++;
            //���ݿ���꣬��Ҫ����һ�����ݿ�
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
            //����r��ָ��
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
            if(s_data.y==r_data.y&&s_prev==s_pointer_data)     s_join = 1;          //�ж�s���������Ƿ���r����ͬ�������ظ���ȡ
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
                    if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
                    {
                        perror("Writing Block Failed!\n");
                        return -1;
                    }
                    printf("ע:���д�����:%d\n",write_blk+blk_count);
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
            //���ݿ���꣬��Ҫ����һ�����ݿ�
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
        if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        printf("ע:���д�����:%d\n",write_blk+blk_count);
        disk_blk = getNewBlockInBuffer(&buf);
        blk_count++;
    }
    freeBlockInBuffer(disk_blk,&buf);
    freeBuffer(&buf);
    printf("S��R�Ĳ�������%d��Ԫ��\n",union_num);
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
    unsigned char *read_s_blk;                             //��ȡs���ݴ��̿��
    unsigned char *read_r_blk;                             //��ȡr���ݴ��̿��
    unsigned char *disk_blk;                               //��д����̿�
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
    int s_pointer_data,r_pointer_data;                          //����s,rָ���Ӧ������
    memcpy(str,read_s_blk,4);                                   //s,rָ���ʼ��Ϊ���е�һ������
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
                    if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
                    {
                        perror("Writing Block Failed!\n");
                        return -1;
                    }
                    printf("ע:���д�����:%d\n",write_blk+blk_count);
                    disk_blk = getNewBlockInBuffer(&buf);
                    blk_count++;
                    count = 0;
                }
            }
            s_count++;
            s_join = 0;
            //���ݿ���꣬��Ҫ����һ�����ݿ�
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
            //��ǰ��ǰs����ֵ���������ӣ���ǰ�ƺ�s����ֵ����,��ʱ��Ҫ��r�����
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
            //����Ϊ��������������ָ���Ӧ��ֵ
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
            //���ݿ���꣬��Ҫ����һ�����ݿ�
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
            //���ݿ���꣬��Ҫ����һ�����ݿ�
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
        if (writeBlockToDisk(disk_blk, write_blk+blk_count, &buf) != 0)  //writeBlockToDisk�����к���free�ڴ��Ĳ�������˲�����ʹ��freeBlockInBuffer����
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        printf("ע:���д�����:%d\n",write_blk+blk_count);
        disk_blk = getNewBlockInBuffer(&buf);
        blk_count++;
    }
    freeBlockInBuffer(disk_blk,&buf);
    freeBuffer(&buf);
    printf("S��R�Ĳ����%d��Ԫ��\n",difference_num);
}
void task1(void)
{
    printf("=================================\n");
    printf("��������������ѡ���㷨S.C=128\n");
    printf("=================================\n");
    Linear_Search();
}

void task2(void)
{
    printf("=================================\n");
    printf("���׶ζ�·�鲢�����㷨(TPMMS)\n");
    printf("=================================\n");
    TPMMS(1,16,301);
    TPMMS(17,48,317);
}

void task3(void)
{
    printf("=================================\n");
    printf("���������Ĺ�ϵѡ���㷨S.C=128\n");
    printf("=================================\n");
    int index_blk_num = createIndex(317,348,401);
    SelectByIndex(401,index_blk_num,128,450);
}

void task4(void)
{
    printf("=================================\n");
    printf("��������������㷨\n");
    printf("=================================\n");
    Sort_Merge_Join(317,348,301,316,500);
}

void task5_1(void)
{
    printf("=================================\n");
    printf("��������ļ��ϵĽ�����\n");
    printf("=================================\n");
    intersection(317,348,301,316,650);
}

void task5_2(void)
{
    printf("=================================\n");
    printf("��������ļ��ϵĲ�����\n");
    printf("=================================\n");
    UnionSet(317,348,301,316,700);
}

void task5_3(void)
{
    printf("=================================\n");
    printf("��������ļ��ϵĲ�����\n");
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
