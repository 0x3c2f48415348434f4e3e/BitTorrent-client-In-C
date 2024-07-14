# BitTorrent Client
According to my research a bit torrent is a peer-to-peer protocol that
enables the distribution of files.
#BitTorrent specification
**https://wiki.theory.org/BitTorrentSpecification**

# Algorithm
1. Read torrent file
2. Extract torrent Data
3. Serialise data and place into a neat structure
4. 

# Current difficulty
A difficult i have foud is related to reading the torrent file. It has come to my attention that a torrent file may contain 'null-terminating' characters and this makes it much harder to process. In order to fix this issue, i will treat the stream not as a string but rather as individual characters of data. Once that is done i will create a new string placeholder and anywhere anull terminating character is found, i will replace it with an arbitary character such as 1.

Another difficult i have faced was within the actual serialization of the data. After many trial and errors, i have been able to produce a decent working code, but may not be the most efficient.
