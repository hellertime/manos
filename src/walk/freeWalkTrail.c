void freeWalkTrail(WalkTrail* t) {
    ASSERT_NOT_BADPTR(t);
    free(t);
}
