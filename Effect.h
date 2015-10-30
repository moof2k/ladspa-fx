
#ifndef _H_EFFECT
#define _H_EFFECT


class Effect
{
public:
    Effect()
    {
    }

    virtual ~Effect()
    {
    }

    virtual void activate() = 0;

    virtual void run(float **audio_in, float **audio_out, unsigned long samples, unsigned int channels) = 0;

private:

};

#endif // _H_EFFECT
