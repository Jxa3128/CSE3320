//This is a user-defined header file
//Jorge Avila & Jennifer Ulloa

#include <stdio.h>

// contains 3 5 2 2 0 4 6 4 4 1 3 4 5 0 4 5 6 2 0 0 1
//prototypes
int FIFO(int *, int, int);
int LRU(int *, int, int);
int full(int *,int);
int MRU (int *, int,int);



//functions for algorithmss
//points to the first element of Array
int FIFO(int * A, int N, int PAGE_TABLE_SIZE)
{
  int i= 0, j= 0 , k=0, page_faults = 0;
  int temp[PAGE_TABLE_SIZE];

  for(i = 0; i < PAGE_TABLE_SIZE; i++)
  {
    temp[i] = -1;
  }
  for(i = 0; i < N; i++)
  {
    j = 0;
    for(k = 0; k < PAGE_TABLE_SIZE; k++)
    {
      if(A[i] == temp[k])
      {
        j++;
        page_faults--;
      }
    }
    page_faults++;
    if((page_faults <= PAGE_TABLE_SIZE) && (j == 0))
    {
      temp[i] = A[i];
    }
    else if(j == 0)
    {
      temp[(page_faults - 1) % PAGE_TABLE_SIZE] = A[i];
    }
  }
  return page_faults;
  }
int LRU (int * A, int N, int PAGE_TABLE_SIZE)
{
  int temp[PAGE_TABLE_SIZE];
  int page_faults = 0;
  int full_slots;
  int i,j,r,n;
  int tempvalue;
  int position = 0;
  int temp3[PAGE_TABLE_SIZE]; //sorted array to find # that is the LRU
  int temp2[PAGE_TABLE_SIZE]; //this one will keep track of doubles

  temp[position]=A[position];

  //printf("\n\t%d\n",q[k]);
  page_faults++;
  position++;
  //traverse all the numbers in our array
  for(i=0;i<N;i++)
  {
    full_slots=0;
    //Check if we already have that new value in our listing of pages
    //if we do not have it increment how many pages we have full
    for(j=0;j<PAGE_TABLE_SIZE;j++)
    {
      if(A[i]!=temp[j])
      {
        full_slots++;
      }
    }
    //if all our slots are full
    if(full_slots==PAGE_TABLE_SIZE)
    {
      page_faults++;
      //if we still don't have 3 then fill up the first three spots of
      //our temp array
      if(position<PAGE_TABLE_SIZE)
      {
        temp[position]=A[i];
        position++;
      }
      //traverse our array backwards to check what numbers we have had up to
      //this point and compare it to the current number
      else
      {
        for(r=0;r<PAGE_TABLE_SIZE;r++)
        {
          temp2[r]=0;
          for(j=i-1;j<N;j--)
          {
            if(temp[r]!=A[j]) //
            temp2[r]++;
            else
            {
              break; //if it does equal what we have there is no point in going on
            }
          }
        }
        for(r=0;r<PAGE_TABLE_SIZE;r++)
        temp3[r]=temp2[r]; //copy on to the third
        for(r=0;r<PAGE_TABLE_SIZE;r++)
        {
          for(j=r;j<PAGE_TABLE_SIZE;j++) //this was j=r at first;
          {
            //sort our temp array to find the LRU by comparing the values by
            //comparing 0 and 1 and so on to find the one with the smallest #
            if(temp3[r]<temp3[j])
            {
              tempvalue=temp3[r]; //what we have save it in a temp value before swap
              temp3[r]=temp3[j]; //since the other one is bigger switch the values
              temp3[j]=tempvalue; //Our new least ecently used value
            }
          }
        }
        for(r=0;r<PAGE_TABLE_SIZE;r++)
        {
          if(temp2[r]==temp3[0])
          temp[r]=A[i];
        }
      }
    }
  }
  return page_faults;
}

int MRU(int * A, int N, int PAGE_TABLE_SIZE)
{
  int i= 0, j= 0 , k=0, page_faults = 0,m=0;
  int temp[PAGE_TABLE_SIZE];
  int position=0;

  for(i = 0; i < PAGE_TABLE_SIZE; i++) //fills table with -1
  {
    temp[i] = -1;
  }
  for(i = 0; i < N; i++)
  {
    j = 0; //signaler
    for(k = 0; k < PAGE_TABLE_SIZE; k++)
    {
        if(A[i] == temp[k]) //if we have it already
        {
          j++;
          temp[k]=A[i]; //this may mess it up
          break; //once it finds out get out
          //page_faults--;
        }
    }
    if((page_faults < PAGE_TABLE_SIZE) && (j == 0))
    {
      for(m=0;m<PAGE_TABLE_SIZE; m++)
      {
        if(temp[m] == -1)
        {
          temp[m] = A[i];
          page_faults++;
          break;
        }
      }
    }
    else if(j==0)
    {
      position = A[i-1];
      for(k=0;k<PAGE_TABLE_SIZE;k++)
      {
        if(temp[k]==position)
        {
          temp[k]= A[i];
          page_faults++;
        }
      }
    }

    }

  return page_faults;
}



int Optimal (int * A, int N, int PAGE_TABLE_SIZE)
{
  int i= 0, j= 0 , k=0, page_faults = 0, m=0;
  int temp[PAGE_TABLE_SIZE];
  int position=0;
  int minimum;
  int nottherenum = 0;
  int numatminposition = 0;
  for(i = 0; i < PAGE_TABLE_SIZE; i++) //fills up
  {
    temp[i] = -1;
  }
  for(i = 0; i < N; i++)
  {
    j = 0; //signaler
    for(k = 0; k < PAGE_TABLE_SIZE; k++)
    {
        if(A[i] == temp[k]) //if we have it signal it so it skips it
        {
          j++;
          break;
        }
    }
    if((i < PAGE_TABLE_SIZE) && (j == 0)) //if not there and we still don't have 3 page faults then add it
    {
      temp[i] = A[i]; //add it to the table
      page_faults++; //since we add it then increment
    }
    /*else if((page_faults < PAGE_TABLE_SIZE) && (j==0))
    {
      for(m=0;m<PAGE_TABLE_SIZE; m++)
      {
        if(temp[m]==-1)
        {
            temp[m] = A[i];
        }
      }
    }*/
    else if(j==0)
    {
      position = i;
      int nummax = -1;
      for(k=0;k<PAGE_TABLE_SIZE;k++)
      {
          for(m=i+1; m<N; m++)
          {
            if(temp[k] == A[m])
            {
                if(m>position)
                {
                  position = m;
                  nummax = k;
                }
                break;
            }
          }
        //we need to save that minimum before going on
        if(m == N)
        {
          position = N;
          nottherenum = temp[k];
          break;
        }
      }
      if(position == N )
      {
        for(m = 0; m<PAGE_TABLE_SIZE; m++)
        {
          if(temp[m] == nottherenum)
          {
            temp[m] = A[i];
            page_faults++;
          }
        }
      }
      else
      {
        numatminposition = A[position];
        for(m = 0; m<PAGE_TABLE_SIZE; m++)
        {
          if(temp[m] == numatminposition)
          {
            temp[m] = A[i];
            page_faults++;

          }
        }
      }
    }
  }
  return page_faults;
}
