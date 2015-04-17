// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_io_process_io_context_h)
#define INCLUDE_evita_io_process_io_context_h

#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "common/win/scoped_handle.h"
#include "evita/dom/public/io_context_id.h"
#include "evita//io/io_context.h"

namespace base {
class Thread;
}

namespace io {

class ProcessIoContext : public IoContext {
  private: domapi::IoContextId context_id_;
  private: std::unique_ptr<base::Thread> gateway_thread_;
  private: common::win::scoped_handle process_;
  private: common::win::scoped_handle stdin_write_;
  private: common::win::scoped_handle stdout_read_;

  public: ProcessIoContext(domapi::IoContextId context_id,
                  const base::string16& command_line,
                  const domapi::OpenProcessDeferred& deferred);
  public: ~ProcessIoContext();

  private: void CloseAndWaitProcess(const domapi::FileIoDeferred& deferred);
  private: uint32_t CloseProcess();
  private: void ReadFromProcess(void* buffer, size_t num_read,
                                const domapi::FileIoDeferred& deferred);
  private: void StartProcess(domapi::IoContextId context_id,
                             const base::string16& command_line,
                             const domapi::OpenProcessDeferred& deferred);
  private: void WriteToProcess(void* buffer, size_t num_read,
                               const domapi::FileIoDeferred& deferred);

  // io::IoContext
  private: virtual void Close(
      const domapi::FileIoDeferred& deferred) override;
  private: virtual void Read(void* buffer, size_t num_read,
                             const domapi::FileIoDeferred& deferred) override;
  private: virtual void Write(void* buffer, size_t num_write,
                             const domapi::FileIoDeferred& deferred) override;

  DISALLOW_COPY_AND_ASSIGN(ProcessIoContext);
};

}  // namespace io

#endif //!defined(INCLUDE_evita_io_process_io_context_h)
