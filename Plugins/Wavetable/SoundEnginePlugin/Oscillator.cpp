#include "Oscillator.h"

void ComplexFFT(AkUInt32 N, AkReal64 *Real, AkReal64 *Imaginary)
{    
    AkInt32 i, j, k, L;            /* indexes */
    AkInt32 M, TEMP, LE, LE1, ip;  /* M = log N */
    AkInt32 NV2, NM1;
    AkReal64 t;               /* temp */
    AkReal64 Ur, Ui, Wr, Wi, Tr, Ti;
    AkReal64 Ur_old;
    
    // if ((N > 1) && !(N & (N - 1)))   // make sure we have a power of 2
    
    NV2 = N >> 1;
    NM1 = N - 1;
    TEMP = N; /* get M = log N */
    M = 0;
    while (TEMP >>= 1) ++M;
    
    /* shuffle */
    j = 1;
    for (i = 1; i <= NM1; i++) {
        if(i<j) {             /* swap a[i] and a[j] */
            t = Real[j-1];     
            Real[j-1] = Real[i-1];
            Real[i-1] = t;
            t = Imaginary[j-1];
            Imaginary[j-1] = Imaginary[i-1];
            Imaginary[i-1] = t;
        }
        
        k = NV2;             /* bit-reversed counter */
        while(k < j) {
            j -= k;
            k /= 2;
        }
        
        j += k;
    }
    
    LE = 1.;
    for (L = 1; L <= M; L++) {            // stage L
        LE1 = LE;                         // (LE1 = LE/2) 
        LE *= 2;                          // (LE = 2^L)
        Ur = 1.0;
        Ui = 0.; 
        Wr = cos(PI32/(AkReal32)LE1);
        Wi = -sin(PI32/(AkReal32)LE1); // Cooley, Lewis, and Welch have "+" here
        for (j = 1; j <= LE1; j++) {
            for (i = j; i <= N; i += LE) { // butterfly
                ip = i+LE1;
                Tr = Real[ip-1] * Ur - Imaginary[ip-1] * Ui;
                Ti = Real[ip-1] * Ui + Imaginary[ip-1] * Ur;
                Real[ip-1] = Real[i-1] - Tr;
                Imaginary[ip-1] = Imaginary[i-1] - Ti;
                Real[i-1]  = Real[i-1] + Tr;
                Imaginary[i-1]  = Imaginary[i-1] + Ti;
            }
            Ur_old = Ur;
            Ur = Ur_old * Wr - Ui * Wi;
            Ui = Ur_old * Wi + Ui * Wr;
        }
    }
}

bool AddTable(WAVETABLE_OSCILLATOR *Oscillator, AkUInt32 Length, AkReal32 *InputSamples, AkReal64 MaxFrequency)
{
    //If there's space in the list, copy to the table in
    if(Oscillator->WavetableCount < MAX_WAVETABLES) 
    {
        AkUInt32 CurrentTable = Oscillator->WavetableCount;

        WAVETABLE Table = {};
        Table.Length = Length;
        Table.FrequencyLimit = MaxFrequency;
        Table.Samples = InputSamples;

        Oscillator->Tables[CurrentTable] = Table;
        ++Oscillator->WavetableCount;
        
        return false;
    }

    //Oscillator is full, stop adding tables
    return true;
}

AkReal32 CreateTable(WAVETABLE_OSCILLATOR *Oscillator, AkUInt32 Length, AkReal64 *Real, AkReal64 *Imaginary, AkReal64 Scale, AkReal64 MaxFrequency) 
{
    //Inverse FFT
    ComplexFFT(Length, Real, Imaginary);
    
    //Calculate the normalisation factor
    if(Scale == 0.0)
    {
        //Loop to find the maximum value
        AkReal64 Max = 0;
        for(AkUInt32 i = 0; i < Length; ++i) 
        {
            AkReal64 Current = fabs(Imaginary[i]);
            
            if(Max < Current)
            {
                Max = Current;
            }
        }

        Scale = (1.0 / Max * .999);
    }
    
    //Normalise the table
    AkReal32 *Table  = 0;
    Table       = new AkReal32 [Length];

    for(AkUInt32 i = 0; i < Length; ++i) 
    {
        Table[i] = Imaginary[i] * Scale;
    }
        
    if(AddTable(Oscillator, Length, Table, MaxFrequency))
    {
        Scale = 0.0;
    }
    
    return Scale;
}

void FillTables(WAVETABLE_OSCILLATOR *Oscillator, AkReal64 *FrequencyReal, AkReal64 *FrequencyImaginary, AkUInt32 TableLength) 
{
    //Initialise DC offset to 0
    FrequencyReal[0] = FrequencyImaginary[0] = 0.0;
    FrequencyReal[TableLength >> 1] = FrequencyImaginary[TableLength >> 1] = 0.0;
    
    //Find the highest harmonic in the table by comparing it's amplitude to the noise floor
    AkUInt32 MaxHarmonic = TableLength >> 1;
    AkReal64 NoiseFloor = 0.000001; //-120dB
    while((fabs(FrequencyReal[MaxHarmonic]) + fabs(FrequencyImaginary[MaxHarmonic]) < NoiseFloor) && MaxHarmonic) 
    {
        --MaxHarmonic;
    }

    //Find the maximum frequency where playback of the table isn't aliasing:
    //Maximum playback rate is 1.0 / (2.0 * MaxHarmonic)
    //Further division by 3.0 to prevent aliasing when approaching the next octave table in the list
    AkReal64 MaxFrequency = 2.0 / 3.0 / MaxHarmonic;
    
    //Each subsequent table is the MaxFrequency * 2.0 where the upper bounds of MaxHarmonic is halved
    AkReal64 *Real       = 0;
    AkReal64 *Imaginary  = 0;
    Real            = new AkReal64 [TableLength];
    Imaginary       = new AkReal64 [TableLength];

    //Scale used for normalisation
    AkReal64 Scale = 0.0;
    while(MaxHarmonic) 
    {
        //Initalise table to 0
        for(AkUInt32 i = 0; i < TableLength; ++i)
        {
            Real[i] = 0.0;
            Imaginary[i] = 0.0;
        }

        //Fill table arrays
        for(AkUInt32 i = 1; i <= MaxHarmonic; ++i) 
        {
            //Copy
            Real[i]         = FrequencyReal[i];
            Imaginary[i]    = FrequencyImaginary[i];
            
            //Mirror the FFT
            AkUInt32 End         = (TableLength - i);
            Real[End]       = FrequencyReal[End];
            Imaginary[End]  = FrequencyImaginary[End];
        }
        
        //Create the table and assign to the oscillator, saving the normalisation scale for the next table
        Scale = CreateTable(Oscillator, TableLength, Real, Imaginary, Scale, MaxFrequency);

        //Update the limits for the next table
        MaxFrequency *= 2;
        MaxHarmonic >>= 1;
    }
}

WAVETABLE_OSCILLATOR *CreateWavetableOscillator(AkReal64 *InputTable, AkUInt32 InputTableLength) 
{
    WAVETABLE_OSCILLATOR *Result = 0;
    Result = new WAVETABLE_OSCILLATOR [1];

    //Split into real and imaginary parts
    //Real      - any float number
    //Imaginary - when squared, give a negative result
    AkReal64 *FrequencyReal      = 0;
    AkReal64 *FrequencyImaginary = 0;
    FrequencyReal           = new AkReal64 [InputTableLength];
    FrequencyImaginary      = new AkReal64 [InputTableLength];

    //Initialise arrays
    for(AkUInt32 i = 0; i < InputTableLength; ++i) 
    {
        FrequencyImaginary[i]   = InputTable[i];
        FrequencyReal[i]        = 0.0;
    }

    //Perform FFT and then fill tables according to the harmonics found
    ComplexFFT(InputTableLength, FrequencyReal, FrequencyImaginary);
	FillTables(Result, FrequencyReal, FrequencyImaginary, InputTableLength);
    
    return Result;
}

AkReal32 Lerp(AkReal32 From, AkReal32 To, AkReal32 Delta)
{
    return (1.0f - Delta) * From + Delta * To;
}

AkReal32 GetSample(WAVETABLE_OSCILLATOR *Oscillator, bool IsTruncated) 
{
    //Get the table that matches the frequency range
    AkUInt32 TableIndex = 0;
    while((Oscillator->PhaseIncrement >= Oscillator->Tables[TableIndex].FrequencyLimit) && (TableIndex < (Oscillator->WavetableCount - 1))) 
    {
        ++TableIndex;
    }
    WAVETABLE *Table = &Oscillator->Tables[TableIndex];

    //Truncation to an integer with no interpolation
    if(IsTruncated)
    {   
        AkUInt32 Truncation = (AkInt32) (Oscillator->PhaseCurrent * Table->Length);
        return Table->Samples[Truncation];
    }

    //Linear interpolation
    AkReal64 Current     = Oscillator->PhaseCurrent * Table->Length;
    AkInt32 Truncation  = Current;
    AkReal64 Fraction    = Current - Truncation;
    AkReal32 Sample1     = Table->Samples[Truncation];
    
    ++Truncation;
    if(Truncation >= Table->Length)
    {
        Truncation = 0;
    }
    AkReal32 Sample2     = Table->Samples[Truncation];
    
    AkReal32 Result      = Lerp(Sample1, Sample2, Fraction);
    return Result;
}

void BuildPresets(AK::IAkPluginMemAlloc *Allocator, AkReal64 **List, AkUInt32 Length)
{
    AkReal64 Step = (TWO_PI32 / Length);
    AkReal64 *SineTable     = (AkReal64 *) Allocator->Malloc(sizeof(AkReal64) * Length);
    AkReal64 *SquareTable   = (AkReal64 *) Allocator->Malloc(sizeof(AkReal64) * Length);
    AkReal64 *UserTable1    = (AkReal64 *) Allocator->Malloc(sizeof(AkReal64) * Length);
    AkReal64 *UserTable2    = (AkReal64 *) Allocator->Malloc(sizeof(AkReal64) * Length);

    AKASSERT(SineTable);
    AKASSERT(SquareTable);
    AKASSERT(UserTable1);
    AKASSERT(UserTable2);

    //Sine preset
    for(AkUInt32 i = 0; i < Length; ++i)
    {
        SineTable[i] = sin(i * Step);
    }

    //Square preset
    for(AkUInt32 i = 0; i < Length; ++i)
    {        
        if(i * Step <= PI32)
        {
            SquareTable[i] = 1.0;
        }

        else
        {
            SquareTable[i] = -1.0;
        }

    }

    for(AkUInt32 i = 0; i < Length; ++i)
    {
        UserTable1[i] = WAVE_TABLE_Mopho_Vox_1[i];
    }


    for(AkUInt32 i = 0; i < Length; ++i)
    {
        UserTable2[i] = WAVE_TABLE_Mopho_Vox_2[i];
    }
    
    List[0] = SineTable;
    List[1] = SquareTable;
    List[2] = UserTable1;
    List[3] = UserTable1;

    Allocator->Free(SineTable);
    Allocator->Free(SquareTable);
    Allocator->Free(UserTable1);
    Allocator->Free(UserTable1);
}

ADSR *ADSRCreate(AK::IAkPluginMemAlloc *Allocator, AkReal32 ControlRate, AkReal32 MaxAmplitude, AkReal32 Attack, AkReal32 Decay, AkReal32 SustainAmplitude, AkReal32 Release)
{
    ADSR *Result = (ADSR *) Allocator->Malloc(sizeof(ADSR));

    if(!Result)
    {
        return 0;
    }

    //Amplitudes
    Result->MaxAmplitude        = MaxAmplitude;
    Result->SustainAmplitude    = SustainAmplitude;

    //Convert to sample counts for linear ramping
    Result->Attack              = Attack * ControlRate;
    Result->Decay               = Decay * ControlRate;
    Result->Release             = Release * ControlRate;
    
    //Durations
    Result->Index               = 0;
    Result->DurationInSamples   = ((Attack + Decay + SustainAmplitude + Release) * ControlRate);
    Result->IsActive            = true;

    return Result;
}

AkReal32 ADSRTick(ADSR *Envelope)
{
    AkReal32 Result = 0.0;

    if(Envelope->IsActive)
    {
        //ADSR finished
        if(Envelope->Index == Envelope->DurationInSamples)
        {
            Envelope->IsActive = false;
            return Result;
        }

        //Attack
        if(Envelope->Index <= Envelope->Attack)
        {
            Result = Envelope->Index * (Envelope->MaxAmplitude / Envelope->Attack);
        }

        //Decay
        else if(Envelope->Index <= (Envelope->Attack + Envelope->Decay))
        {
            Result = ((Envelope->SustainAmplitude - Envelope->MaxAmplitude) / Envelope->Decay) * (Envelope->Index - Envelope->Attack) + Envelope->MaxAmplitude;
        }

        //Sustain
        else if(Envelope->Index <= (Envelope->DurationInSamples - Envelope->Release))
        {
            Result = Envelope->SustainAmplitude;
        }

        //Release
        else if(Envelope->Index > (Envelope->DurationInSamples - Envelope->Release))
        {
            Result = -(Envelope->SustainAmplitude / Envelope->Release) * (Envelope->Index - (Envelope->DurationInSamples - Envelope->Release)) + Envelope->SustainAmplitude;
        }

        Envelope->Index++;
    }

    return Result;
}
