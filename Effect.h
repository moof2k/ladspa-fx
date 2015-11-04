
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

    virtual void run(const float *audio_in[2], float *audio_out[2], unsigned long samples) = 0;

private:

};

#endif // _H_EFFECT
