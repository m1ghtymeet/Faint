#pragma once
#include <glad/glad.h>
#include <iostream>

namespace Moon::Loaders { class AsyncPBO; }
namespace Moon::Rendering {
    struct PBO {
    public:
        void Init(size_t size) {
            if (m_handle != 0) CleanUp();

            m_size = size;
            glGenBuffers(1, &m_handle);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_handle);
            glBufferStorage(GL_PIXEL_UNPACK_BUFFER, size, nullptr,
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
            m_persistentBuffer = (GLubyte*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size,
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT |
                GL_MAP_INVALIDATE_BUFFER_BIT);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            m_syncObj = nullptr;
            m_syncInProgress = false;
        }

        uint32_t GetHandle() const {
			return m_handle;
        }

        GLubyte* GetPersistentBuffer() const {
            return m_persistentBuffer;
        }

        bool IsValid() const {
            return m_handle != 0 && m_persistentBuffer != nullptr;
        }

        void SyncStart() {
            if (m_syncObj)
                glDeleteSync(m_syncObj);
            m_syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            glWaitSync(m_syncObj, 0, GL_TIMEOUT_IGNORED);
            m_syncInProgress = true;
        }

        GLenum GetSynceStatus() {
            GLenum result = glClientWaitSync(m_syncObj, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
            return result;
        }

        std::string GetSyncStatusAsString() {
            if (!m_syncObj) {
                return "NO_SYNC_OBJECT";
            }
            GLenum result = GetSynceStatus();
            switch (result) {
            case GL_ALREADY_SIGNALED:     return "GL_ALREADY_SIGNALED";
            case GL_CONDITION_SATISFIED:  return "GL_CONDITION_SATISFIED";
            case GL_TIMEOUT_EXPIRED:      return "GL_TIMEOUT_EXPIRED";
            case GL_WAIT_FAILED:          return "GL_WAIT_FAILED";
            default:                      return "UNKNOWN_GL_SYNC_STATUS";
            }
        }

        bool UpdateState() {
            if (m_syncObj) {
                //GLenum result = glClientWaitSync(m_syncObj, 0, 0);
                GLenum result = glClientWaitSync(m_syncObj, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
                if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED) {
                    glDeleteSync(m_syncObj);
                    m_syncObj = nullptr;
                    m_syncInProgress = false;
                }
                else {
                    m_syncInProgress = true;
                }
            }
            else {
                m_syncInProgress = false;
            }
            return m_syncInProgress;
        }

        bool IsSyncComplete() const {
            return !m_syncInProgress;
        }

        bool IsSyncInProgress() const {
            return m_syncInProgress;
        }

        void WaitOnFence() {
            if (m_syncObj) {
                while (true) {
                    GLenum result = glClientWaitSync(m_syncObj, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000); // 1ms timeout
                    if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED) {
                        glDeleteSync(m_syncObj);
                        m_syncObj = nullptr;
                        m_syncInProgress = false;
                        break;
                    }
                    if (result == GL_WAIT_FAILED) {
                        m_syncObj = nullptr;
                        m_syncInProgress = false;
                        break;
                    }
                }
            }
        }

        void BeginUpload() {
            WaitOnFence();
            m_syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            m_syncInProgress = true;
        }

        void CleanUp() {
            //if (m_syncObj) {
            //    glDeleteSync(m_syncObj);
            //    m_syncObj = nullptr;
            //}
            //if (m_persistentBuffer) {
            //    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_handle);
            //    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
            //    m_persistentBuffer = nullptr;
            //}
            //if (m_handle != 0) {
            //    glDeleteBuffers(1, &m_handle);
            //    m_handle = 0;
            //}
            //m_size = 0;
            WaitOnFence();
            if (m_handle != 0) {
                if (m_persistentBuffer) {
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_handle);
                    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                    m_persistentBuffer = nullptr;
                }
                glDeleteBuffers(1, &m_handle);
                m_handle = 0;
            }
            m_size = 0;
        }

    private:
        size_t m_size = 0;
        GLubyte* m_persistentBuffer = nullptr;
        uint32_t m_handle = 0;
        GLsync m_syncObj = nullptr;
        bool m_syncInProgress = false;
        int16_t m_customValue = -1;
        friend class Loaders::AsyncPBO;
    };
}