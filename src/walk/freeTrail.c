void freeTrail(Trail* t) {
    ASSERT_NOT_BADPTR(t);
    free(t);
}
