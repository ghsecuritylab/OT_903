

#include "config.h"
#include "HashTable.h"

namespace WTF {

#if DUMP_HASHTABLE_STATS

int HashTableStats::numAccesses;
int HashTableStats::numCollisions;
int HashTableStats::collisionGraph[4096];
int HashTableStats::maxCollisions;
int HashTableStats::numRehashes;
int HashTableStats::numRemoves;
int HashTableStats::numReinserts;

static HashTableStats logger;

static Mutex& hashTableStatsMutex()
{
    AtomicallyInitializedStatic(Mutex&, mutex = *new Mutex);
    return mutex;
}

HashTableStats::~HashTableStats()
{
    // Don't lock hashTableStatsMutex here because it can cause deadlocks at shutdown 
    // if any thread was killed while holding the mutex.
    printf("\nWTF::HashTable statistics\n\n");
    printf("%d accesses\n", numAccesses);
    printf("%d total collisions, average %.2f probes per access\n", numCollisions, 1.0 * (numAccesses + numCollisions) / numAccesses);
    printf("longest collision chain: %d\n", maxCollisions);
    for (int i = 1; i <= maxCollisions; i++) {
        printf("  %d lookups with exactly %d collisions (%.2f%% , %.2f%% with this many or more)\n", collisionGraph[i], i, 100.0 * (collisionGraph[i] - collisionGraph[i+1]) / numAccesses, 100.0 * collisionGraph[i] / numAccesses);
    }
    printf("%d rehashes\n", numRehashes);
    printf("%d reinserts\n", numReinserts);
}

void HashTableStats::recordCollisionAtCount(int count)
{
    MutexLocker lock(hashTableStatsMutex());
    if (count > maxCollisions)
        maxCollisions = count;
    numCollisions++;
    collisionGraph[count]++;
}

#endif

} // namespace WTF
