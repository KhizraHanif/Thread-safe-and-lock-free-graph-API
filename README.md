
 A thread-safe, lock-free edge-labeled directed graph data structure with atomic operations."
CSC 591 Assignment 1: Lock-free, thread-safe graph.
Project description:
This project is built for the first assignment of CSC 591, which is meant to implement a lock-free, thread-safe, edge-labeled, directed graph.
This graph supports the following functions:
 1. `create_graph( std::size_t max_capacity )`. This is a constructor or factory function that creates a new empty graph that can support up to `max_capacity` vertices.
 2. `add_vertex()`. Creates a new isolated vertex in constant time if the graph is not already full. Otherwise it returns some type of failure notification.
 3. `add_edge( std::size_t i, std::size_t j )`. Adds directed edge (i, j) to the graph with an initial edge label equal to zero in at most `O(max_capacity)` time. If the edge already exists, its label is reset to 0.
 4. `inc_label( std::size_t i, std::size_t j, std::size_t increment )`. Increases the label on edge (i,j) by `increment` in at most `O(max_capacity)` time
 5. `dec_label( std::size_t i, std::size_t j, std::size_t decrement )`. Decreases the label on edge (i,j) by `decrement` in at most `O(max_capacity)` time if the label is at least `decrement` to begin with; otherwise it returns some type of failure notification
 6. `are_connected( std:size_t i, std::size_t j )`. Returns true if the edge (i,j) exists or false otherwise. Runs in at most `O(|E|)` time.
 7. `is_reachable( std::size_t source, std::size_t destination )`. Returns true if a path exists from `source` to destination` or false otherwise. Runs in at most `O(|E|)` time.
 8. `shortest_path_len( std::size_t source, std::size_t destination )`. Returns the length of the shortest path from `source` to `destination`, or some indication that no path exists.  Runs in at most `O(|E|)` time.
 9. `get_diameter()`. This returns the diameter of the graph and runs in at most `O(|E|)` time.
It should be noted that, this project doesn't support the function of deleting edges or deleting nodes, as such functions may make it a lot more complex to maintain a consistent state among threads. We wish to perfect this data structure in the future. 
Techniques used:
To ensure that the data structure is thread-safe, we use atomic variables for the address of nodes in linked lists, as well as the label of edges. We then use atomic instructions such as CAS and fetch_add operations to modify variables in these classes.
To support the quiry functions, we conducted a BFS search.
How to run this project:
We assume simple comppling and running would be smooth in a Mac envorinment. In a windows system, it is better to use g++ to complie the source code instead of gcc. 
It is also possible to encounter some minor syntax issues regarding on certain instructions. We don't expect other changes are required to run this project.

Credits:
 
This is the collaborative work done by Jax Wu, Khizra Hanif, and Harper Yan. 

References 
The following literature and websites are used as references in our process of doing this project. 
1. https://www.techiedelight.com/graph-implementation-using-stl/
2. https://www.freecodecamp.org/news/how-to-write-a-good-readme-file/
Chatterjee, B., Peri, S., Sa, M., & Singhal, N. (2018). A Simple and Practical Concurrent Non-blocking Unbounded Graph with Reachability Queries. ArXiv. /abs/1809.00896.
