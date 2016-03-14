#define DATA_SIZE 4
#define data_t float

float input[DATA_SIZE*DATA_SIZE] = 
{
    0.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 1.0, 0.0,
    0.0, 1.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 0.0
};

short mask[DATA_SIZE*DATA_SIZE] = 
{
    1, 0, 1, 0,
    1, 0, 1, 0,
    1, 0, 1, 0,
    1, 0, 1, 0,
};

float verify_data[DATA_SIZE*DATA_SIZE] = 
{
    0.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 4.0, 0.0,
    0.0, 1.0, 4.0, 0.0,
    0.0, 0.0, 0.0, 0.0
};


