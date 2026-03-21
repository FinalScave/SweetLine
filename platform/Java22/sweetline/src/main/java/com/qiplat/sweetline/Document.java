package com.qiplat.sweetline;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;

/**
 * Managed document with incremental update support.
 * <p>
 * Implements {@link AutoCloseable} for deterministic native resource release.
 */
public class Document implements AutoCloseable {

    private final MemorySegment nativeHandle;
    private boolean closed = false;

    /**
     * Create a managed document.
     *
     * @param uri     Document URI
     * @param content Document content
     */
    public Document(String uri, String content) {
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment uriSeg = arena.allocateFrom(uri);
            MemorySegment contentSeg = arena.allocateFrom(content);
            this.nativeHandle = (MemorySegment) SweetLineNative.sl_create_document.invoke(uriSeg, contentSeg);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to create document", e);
        }
    }

    /**
     * Get the native handle for internal use.
     */
    MemorySegment handle() {
        return nativeHandle;
    }

    @Override
    public void close() {
        if (!closed) {
            closed = true;
            try {
                SweetLineNative.sl_free_document.invoke(nativeHandle);
            } catch (Throwable e) {
                throw new RuntimeException("Failed to free document", e);
            }
        }
    }
}
