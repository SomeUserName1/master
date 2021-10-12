import java.io.IOException;
import java.nio.file.Path;
import java.time.Duration;
import java.time.Instant;
import java.util.Iterator;

import org.neo4j.configuration.GraphDatabaseSettings;
import org.neo4j.dbms.api.DatabaseManagementService;
import org.neo4j.dbms.api.DatabaseManagementServiceBuilder;
import org.neo4j.graphdb.*;

import static org.neo4j.configuration.GraphDatabaseSettings.DEFAULT_DATABASE_NAME;
import static org.neo4j.io.fs.FileUtils.deleteDirectory;

public class Benchmark {
    private static final Path databaseDirectory = Path.of("target/neo4j-db");
    GraphDatabaseService graphDb;
    private DatabaseManagementService managementService;
    static int n_calls_per_fn = 15_000;

    public static void main(final String[] args) throws IOException {
        Benchmark bench = new Benchmark();

        // Benchmark create nodes and relationships
        bench.createDb();
        bench.createNodes(true);
        bench.createRels(true);

        // Benchmark read nodes and relationships
        bench.readNodes();
        bench.readRelationships();

        bench.updateNodes();
        bench.updateRelationships();

        // Remove and recreate all nodes to "undo" the updates
        bench.deleteNodes(false);
        bench.createNodes(false);
        bench.createRels(false);

        // Benchmark delete nodes and relationships
        bench.deleteNodes(true);

        // Recreate the just deleted records
        bench.createNodes(false);
        bench.createRels(false);

        bench.deleteRelationships();

        // Create new nodes and relationships for expand and get nodes
        bench.createRels(false);

        // benchmark expand and get nodes
       bench.getNodes();
       bench.getRelationships();
       bench.expand();

        bench.shutDown();
    }

    void createDb() throws IOException {
        deleteDirectory(databaseDirectory);

        managementService = new DatabaseManagementServiceBuilder(databaseDirectory)
            .setConfig(GraphDatabaseSettings.pagecache_memory, "81920")
            .build();
        graphDb = managementService.database(DEFAULT_DATABASE_NAME);
        registerShutdownHook(managementService);


    }

    void createNodes(boolean time) {
        Instant start = null;
        Instant end = null;
        long total = 0;

        try (Transaction tx = graphDb.beginTx()) {
            for (int i = 0; i < n_calls_per_fn; ++i) {
                if (time) {
                    start = Instant.now();
                }
                tx.createNode(Label.label(Integer.toString(i)));
                if (time) {
                   end = Instant.now();
                   total += Duration.between(start, end).toNanos();
                }
            }
            tx.commit();
        }

        if (time) {
            System.out.println("Creating 20K Nodes took " + Duration.ofNanos(total) + ". Average call " +
                    "takes " + (total / n_calls_per_fn) / 1000 + "mu s \n");

        }
    }

    void createRels(boolean time) {
        Instant start = null;
        Instant end = null;
        long total = 0;

        try (Transaction tx = graphDb.beginTx()) {
            Node fst = tx.findNodes(Label.label(Integer.toString(0))).next();
            Iterator<Node> nodes = tx.getAllNodes().stream().iterator();
            for (int i = 0; i < n_calls_per_fn; ++i) {
                if (time) {
                    start = Instant.now();
                }
                fst.createRelationshipTo(nodes.next(), RelationshipType.withName(Integer.toString(i)));
                if (time) {
                   end = Instant.now();
                   total += Duration.between(start, end).toNanos();
                }

            }
            tx.commit();
        }
        if (time) {
            System.out.println("Creating 20K Relationships took " + Duration.ofNanos(total) + ". Average call " +
                    "takes " + (total / n_calls_per_fn) / 1000 + "mu s \n");
        }
    }

    void readNodes() {
        Instant start = null;
        Instant end = null;
        long total = 0;

        try (Transaction tx = graphDb.beginTx()) {
            for (int i = 0; i < n_calls_per_fn; ++i) {
                start = Instant.now();
                tx.findNodes(Label.label(Integer.toString(i))).next();
                end = Instant.now();
                total += Duration.between(start, end).toNanos();
            }

            tx.commit();
        }

        System.out.println("Finding 20K Nodes by label took " +  Duration.ofNanos(total) + ". Average call " +
                "takes " + (total / n_calls_per_fn) / 1000 + "mu s \n");
    }

    void readRelationships() {
         Instant start = null;
        Instant end = null;
        long total = 0;

        try (Transaction tx = graphDb.beginTx()) {
            for (int i = 0; i < n_calls_per_fn; ++i) {
                start = Instant.now();
                tx.findRelationships(RelationshipType.withName(Integer.toString(i)));
                end = Instant.now();
                total += Duration.between(start, end).toNanos();
            }
            tx.commit();
        }
        System.out.println("Finding 20K Edges by label took " +  Duration.ofNanos(total) + ". Average call " +
                "takes " + (total / n_calls_per_fn) / 1000 + "mu s \n");
    }

    void updateNodes() {
         Instant start = null;
        Instant end = null;
        long total = 0;

        try (Transaction tx = graphDb.beginTx()) {
            Node node;
            for (int i = 0; i < n_calls_per_fn; ++i) {
                node = tx.findNodes(Label.label(Integer.toString(i))).next();
                start = Instant.now();
                node.removeLabel(Label.label(Integer.toString(i)));
                node.addLabel(Label.label(Integer.toString(n_calls_per_fn - i - 1)));
                end = Instant.now();
                total += Duration.between(start, end).toNanos();
            }
            tx.commit();
        }
        System.out.println("Updating 20K Nodes took " + Duration.ofNanos(total) + ". Average call " +
                "takes " + (total / n_calls_per_fn) / 1000 + "mu s \n");
    }

    void updateRelationships() {
        System.out.println("Relationship can not be updated besides their properties!\n");
    }

    void deleteNodes(boolean time) {
        Instant start = null;
        Instant end = null;
        long total = 0;
        try (Transaction tx = graphDb.beginTx()) {
            Node node;
            Iterator<Relationship> rels;
            for (int i = 0; i < n_calls_per_fn; ++i) {
                node = tx.findNodes(Label.label(Integer.toString(i))).next();
                if (time) {
                    start = Instant.now();
                }

                rels = node.getRelationships().iterator();

                while (rels.hasNext()) {
                    rels.next().delete();
                }
                node.delete();

                if (time) {
                    end = Instant.now();
                    total += Duration.between(start, end).toNanos();
                }
            }

            tx.commit();
        }

        if (time) {
        System.out.println("Deleting 20K Nodes took " + Duration.ofNanos(total) + ". Average call " +
                "takes " + (total / n_calls_per_fn) / 1000 + "mu s \n");
        }
    }

    void deleteRelationships() {
        Instant start = null;
        Instant end = null;
        long total = 0;

        Relationship rel;
        try (Transaction tx = graphDb.beginTx()) {
            for (int i = 0; i < n_calls_per_fn; ++i) {
                start = Instant.now();
                rel = tx.findRelationships(RelationshipType.withName(Integer.toString(i))).next();
                rel.delete();
                end = Instant.now();
                total += Duration.between(start, end).toNanos();
            }

            tx.commit();
        }
        System.out.println("Deleting 20K Relationships took " + Duration.ofNanos(total) + ". Average call " +
                "takes " + (total / n_calls_per_fn) / 1000 + "mu s \n");
    }

    void getNodes() {
        Instant start = null;
        Instant end = null;
        long total = 0;

        try (Transaction tx = graphDb.beginTx()) {
            Iterator<Node> nodes;
            for (int i = 0; i < n_calls_per_fn; ++i) {
                start = Instant.now();
                nodes = tx.getAllNodes().stream().iterator();

                while (nodes.hasNext()) {
                    nodes.next();
                }
                end = Instant.now();
                total += Duration.between(start, end).toNanos();
            }

            tx.commit();
        }
        System.out.println("Calling GetNodes 20K times took " + Duration.ofNanos(total) + ". Average call " +
                "takes " + (total / n_calls_per_fn) / 1000  + "mu s \n");
    }

    void getRelationships() {
        Instant start = null;
        Instant end = null;
        long total = 0;

        try (Transaction tx = graphDb.beginTx()) {

            Iterator<Relationship> rels;
            for (int i = 0; i < n_calls_per_fn; ++i) {
                start = Instant.now();
                rels = tx.getAllRelationships().stream().iterator();

                while (rels.hasNext()) {
                    rels.next();
                }
                end = Instant.now();
                total += Duration.between(start, end).toNanos();
            }
            tx.commit();
        }
        System.out.println("Calling GetRelationships 20K times took " + Duration.ofNanos(total) + ". Average call " +
                "takes " + (total / n_calls_per_fn) / 1000  + "mu s \n");
    }

    void expand() {
        Instant start = null;
        Instant end = null;
        long total = 0;

        try (Transaction tx = graphDb.beginTx()) {
            Node node = tx.findNodes(Label.label(Integer.toString(0))).next();
            Iterator<Relationship> rels;
            for (int i = 0; i < n_calls_per_fn; ++i) {
                start = Instant.now();
                rels = node.getRelationships().iterator();

                while (rels.hasNext()) {
                    rels.next();
                }
                end = Instant.now();
                total += Duration.between(start, end).toNanos();
            }

            tx.commit();
        }
        System.out.println("Calling expand 20K times took " + Duration.ofNanos(total) + ". Average call " +
                "takes " + (total / n_calls_per_fn) / 1000  + "mu s \n");
    }

    void shutDown() {
        System.out.println();
        managementService.shutdown();
    }

    private static void registerShutdownHook(final DatabaseManagementService managementService) {
        Runtime.getRuntime().addShutdownHook(new Thread(managementService::shutdown));
    }
}
