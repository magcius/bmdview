#ifndef BMD_CLOCK_H
#define BMD_CLOCK_H BMD_CLOCK_H

template<int T = 20>
class Clock
{
 public:
   Clock() : m_numValues(0), m_sum(0.0f) { }

   void addValue(float val)
   {
     m_sum += val;
     if(m_numValues < T)
     {
       m_values[m_numValues] = val;
       ++m_numValues;
     }
     else
     {
       m_sum -= m_values[0];
       for(int i = 0; i < T - 1; ++i)
         m_values[i] = m_values[i + 1];
       m_values[T - 1] = val;
     }
   }

   float getAverage()
   {
     if(m_numValues == 0)
       return 0;

     float sum = 0.0f;
     for(int i = 0; i < m_numValues; ++i)
       sum += m_values[i];

     return sum/m_numValues;
   }

 private:
   float m_values[T];
   int m_numValues;
   float m_sum;
};



#endif //BMD_CLOCK_H
