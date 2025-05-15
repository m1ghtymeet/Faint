using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace Faint.Net
{
    public class Util
    {
        public static float SMALL_NUMBER = (float)9.99999993922529e-9;

        public static float FInterpTo(float current, float target, float deltaTime, float interpSpeed)
        {
            if (interpSpeed <= 0.0f)
                return target;

            float dist = target - current;

            if (dist * dist < SMALL_NUMBER)
                return target;

            float deltaMove = dist * Math.Max(0.0f, Math.Min(deltaTime * interpSpeed, 1.0f));

            return current + deltaMove;
        }
    }
}