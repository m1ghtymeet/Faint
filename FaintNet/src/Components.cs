using Coral.Managed.Interop;
using Faint.Net;
using System.Numerics;

namespace Faint.Net
{
    public class IComponent
    {
        public int EntityID { get; protected set; }
    }
    public class NameComponent : IComponent
    {
        internal static unsafe delegate*<int, NativeString> GetNameIcall;

        public NameComponent(int entityId)
        {
            EntityID = entityId;
        }

        public string Name
        {
            get
            {
                unsafe { return GetNameIcall(EntityID).ToString(); }
            }
            set
            {

            }
        }
    }
    public class TransformComponent : IComponent
    {
        internal static unsafe delegate*<int, NativeArray<float>> GetGlobalPositionIcall;
        internal static unsafe delegate*<int, NativeArray<float>> GetPositionIcall;
        internal static unsafe delegate*<int, NativeArray<float>> GetGlobalRotationIcall;
        internal static unsafe delegate*<int, NativeArray<float>> GetRotationIcall;
        internal static unsafe delegate*<int, float, float, float, void> SetPositionIcall;
        internal static unsafe delegate*<int, float, float, float, void> SetGlobalRotationIcall;
        internal static unsafe delegate*<int, float, float, float, void> SetRotationIcall;
        internal static unsafe delegate*<int, float, float, float, void> RotateIcall;

        public TransformComponent(int entityId)
        {
            EntityID = entityId;
        }

        public void Rotate(float x, float y, float z)
        {
            unsafe { RotateIcall(EntityID, x, y, z); }
        }

        public Vector3 Rotation
        {
            get
            {
                unsafe
                {
                    NativeArray<float> result = GetRotationIcall(EntityID);
                    return new Vector3(result[0], result[1], result[2]);
                }
            }
            set { unsafe { SetRotationIcall(EntityID, value.x, value.y, value.z); } }
        }
        public Vector3 GlobalRotation
        {
            get
            {
                unsafe
                {
                    NativeArray<float> result = GetGlobalRotationIcall(EntityID);
                    return new Vector3(result[0], result[1], result[2]);
                }
            }
            set
            {
                unsafe { SetGlobalRotationIcall(EntityID, value.x, value.y, value.z); }
            }
        }

        public void SetLocalPosition(float x, float y, float z)
        {
            unsafe { SetPositionIcall(EntityID, x, y, z); }
        }
        

        public Vector3 LocalPosition
        {
            get
            {
                unsafe
                {
                    NativeArray<float> result = GetPositionIcall(EntityID);
                    return new Vector3(result[0], result[1], result[2]);
                }
            }
            set
            {
                unsafe { SetPositionIcall(EntityID, value.x, value.y, value.z); }
            }
        }
        public Vector3 GetLocalPosition()
        {
            return LocalPosition;
        }

        public Vector3 GlobalPosition
        {
            get
            {
                unsafe
                {
                    NativeArray<float> result = GetGlobalPositionIcall(EntityID);
                    return new Vector3(result[0], result[1], result[2]);
                }
            }
            set { }
        }
    }
    public class CameraComponent : IComponent
    {
        internal static unsafe delegate*<int, float> GetFieldOfViewIcall;
        internal static unsafe delegate*<int, float> GetNearPlaneIcall;
        internal static unsafe delegate*<int, float> GetFarPlaneIcall;
        internal static unsafe delegate*<int, NativeArray<float>> GetDirectionIcall;
        internal static unsafe delegate*<int, NativeArray<float>> GetCameraUpIcall;
        internal static unsafe delegate*<int, NativeArray<float>> GetCameraRightIcall;

        public CameraComponent(int entityId)
        {
            EntityID = entityId;
        }

        public float FOV
        {
            get
            {
                unsafe { return GetFieldOfViewIcall(EntityID); }
            }
        }
        public float NearPlane
        {
            get
            {
                unsafe { return GetNearPlaneIcall(EntityID); }
            }
        }
        public float FarPlane
        {
            get
            {
                unsafe { return GetFarPlaneIcall(EntityID); }
            }
        }

        public Vector3 Direction
        {
            get
            {
                unsafe
                {
                    NativeArray<float> result = GetDirectionIcall(EntityID);
                    return new Vector3(result[0], result[1], result[2]);
                }
            }
        }

        public Vector3 GetUp()
        {
            unsafe
            {
                NativeArray<float> result = GetCameraUpIcall(EntityID);
                return new Vector3(result[0], result[1], result[2]);
            }
        }

        public Vector3 GetRight()
        {
            unsafe
            {
                NativeArray<float> result = GetCameraRightIcall(EntityID);
                return new Vector3(result[0], result[1], result[2]);
            }
        }
    }
    public class ModelComponent : IComponent
    {
        public ModelComponent(int entityId)
        {
            EntityID = entityId;
        }

        public string Mesh { get; set; }
    }
    public class RigidbodyComponent : IComponent
    {
        public RigidbodyComponent(int entityId)
        {
            EntityID = entityId;
        }

        public string Mass { get; set; }
    }
    public class BoxCollider : IComponent
    {
        public BoxCollider(int entityId)
        {
            EntityID = entityId;
        }

        public string Scale { get; set; }
    }
    public class CharacterControllerComponent : IComponent
    {
        internal static unsafe delegate*<int, float, float, float, void> MoveAndSlideIcall;
        internal static unsafe delegate*<int, bool> IsOnGroundedIcall;

        public CharacterControllerComponent(int entityId)
        {
            EntityID = entityId;
        }

        public void Move(Vector3 velocity)
        {
            unsafe { MoveAndSlideIcall(EntityID, velocity.x, velocity.y, velocity.z); }
        }

        public bool IsOnGrounded()
        {
            unsafe { return IsOnGroundedIcall(EntityID); }
        }
    }

    public class AudioComponent : IComponent
    {
        internal static unsafe delegate*<int, string, float, void> PlayAudioByPathIcall;
        internal static unsafe delegate*<int, float, void> PlayAudioIcall;
        internal static unsafe delegate*<int, bool> IsPlayingIcall;
        internal static unsafe delegate*<int, string> GetAudioPathIcall;

        public AudioComponent(int entityId)
        {
            EntityID = entityId;
        }

        public string AudioPath
        {
            get
            {
                unsafe { return GetAudioPathIcall(EntityID); }
            }
            set
            {
                unsafe { }
            }
        }

        public void PlayAudioByPath(string path, float volume)
        {
            unsafe { PlayAudioByPathIcall(EntityID, path, volume); }
        }
        public void PlayAudio(float volume)
        {
            unsafe { PlayAudioIcall(EntityID, volume); }
        }

        public float Volume
        {
            get
            {
                unsafe { return 1.0f; }
            }
            set
            {
                unsafe { }
            }
        }
        public bool Loop { get; set; }
        public bool Spatialized { get; set; }
        public bool Playing
        {
            get
            {
                unsafe { return IsPlayingIcall(EntityID); }
            }
            set
            {
                unsafe { PlayAudioIcall(EntityID, Volume); }
            }
        }
    }
}