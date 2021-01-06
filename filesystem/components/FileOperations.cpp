
#include "FileOperations.hpp"

FileOperations::FileOperations(FileSystemController& fileSystemController) : fileSystemController(
        fileSystemController) {
    this->pathContext = std::make_unique<PathContext>(*this);
}

void FileOperations::printCurrentFolderItems() {
    // iterace pres vsechny folder items a vytisteni jejich jmena
    for (auto& item : pathContext->folderItems) {
        auto node = fileSystemController.getINodeFromAddress(item.nodeAddress);
        std::cout << (node.isFolder() ? "+" : "-") << item.getItemName() << std::endl;
    }
}

void FileOperations::listItems(const std::string& path) {
    auto fsPath = FileSystemPath(path);

    // pokud je aktualni cesta prazdna (tzn pouze prikaz ls) vypiseme aktualni adresar
    if (fsPath.size() == 0) {
        printCurrentFolderItems();
        return;
    }

    // Pro zjisteni predmetu jinde nez v aktualni slozce potrebujeme zmenit absolutePath,
    // tzn je potreba vytvorit kopii, kterou pote zpetne nahradime aby se uzivatel nevyskytl na miste, ktere chtel
    // zobrazit misto toho, kde se nachazel
    auto absolutePath = pathContext->absolutePath; // kopie absolutni cesty

    try {
        pathContext->moveTo(fsPath);
        // nacteme predmety a vytiskneme
        pathContext->loadItems();
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException("PATH NOT FOUND (neexistujici adresar)");
    }

    // vratime zpet puvodni cestu a nacteme predmety
    printCurrentFolderItems();
    restorePathContextState(absolutePath);
}

void FileOperations::makeDirectory(const std::string& path) {
    auto fsPath = FileSystemPath(path);

    // ulozime aktualni cestu, protoze se v kontextu muzeme presunout
    auto absolutePath = pathContext->absolutePath;

    auto folderName = fsPath.back(); // nazev souboru je posledni prvek objektu FileSystemPath
    fsPath.popBack(); // odstranime nazev slozky, protoze tam se presouvat nebudeme

    try {
        pathContext->moveTo(fsPath); // presuneme se do aktualni cesty
    }
    catch (FSException& exception) {
        restorePathContextState(absolutePath);
        throw FSException("PATH NOT FOUND (neexistuje zadana cesta)");
    }

    pathContext->loadItems(); // nacteme soubory
    if (pathContext->getFolderItemIndex(folderName) != -1) {
        restorePathContextState(absolutePath);
        throw FSException("EXISTS (nelze zalozit, jiz existuje)");
    }

    // ziskani inode rodice (slozky) a adresy rodice
    auto parentNode = pathContext->absolutePath.back();
    auto parentNodeAddress = fileSystemController.getINodeAddress(parentNode);

    // pokud je slozka plna hodi exception (coz asi ale neni realisticke pro tento pripad)
    if (parentNode.isFolderFull()) {
        restorePathContextState(absolutePath);
        throw FSException("Error, specified folder is full");
    }

    auto folderNode = fileSystemController.getFreeINode(true); // ziskani nove INode pro slozku
    auto folderNodeAddress = fileSystemController.getINodeAddress(folderNode); // ziskani adresy

    try {
        FolderItem::validateFolderName(folderName); // validace jmena slozky, jinak vyhodi exception
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException(ex.what());
    }
    auto parentNodeFolderItem = FolderItem(folderName, folderNodeAddress); // folder item v parent node

    try {
        // pridani "." a ".."
        fileSystemController.linkFolderToParent(folderNode, folderNodeAddress, parentNodeAddress);
    }
    catch (FSException& ex) { // neni misto
        fileSystemController.reclaimINode(folderNode);
        restorePathContextState(absolutePath);
        throw FSException("Error, not enough space to create new folder");
    }

    try {
        fileSystemController.appendFolderItem(parentNode, parentNodeFolderItem); // pridani predmetu
    } catch (FSException& ex) { // neni misto
        restorePathContextState(absolutePath);
        fileSystemController.reclaimINode(folderNode);
        throw FSException("Error, not enough space to create new folder");
    }

    restorePathContextState(absolutePath);
    std::cout << "OK" << std::endl;
}

void FileOperations::restorePathContextState(const std::vector<INode>& absolutePath) {
    pathContext->absolutePath = absolutePath;
    pathContext->refresh();
}

void FileOperations::changeDirectory(const std::string& path) {
    auto fsPath = FileSystemPath(path);
    auto absolutePath = pathContext->absolutePath; // kopie pro pripad ze se zmena slozky nezdari

    try {
        pathContext->moveTo(fsPath); // presun do slozky
    }
    catch (FSException& ex) { // vytiskneme chybu
        restorePathContextState(absolutePath);
        std::cout << ex.what() << std::endl;
    }

    pathContext->loadItems(); // nacteni predmetu
    std::cout << "OK" << std::endl;
}

std::string FileOperations::getFolderName(uint64_t nodeAddress, const std::vector<FolderItem>& folderItems) {
    for (auto& folderItem : folderItems) {
        if (folderItem.getNodeAddress() == nodeAddress) {
            return folderItem.getItemName();
        }
    }

    throw FSException("Error this node does not exist in the path"); // debug
}

void FileOperations::printCurrentPath() {
    if (pathContext->absolutePath.size() == 1) {
        std::cout << "/" << std::endl;
        return;
    }

    auto path = std::vector<std::string>();
    path.emplace_back(""); // pro "/" na zacatku
    for (auto i = 1; i < pathContext->absolutePath.size(); i++) {
        auto folderItems = fileSystemController.getFolderItems(pathContext->absolutePath[i - 1]);
        auto nodeAddress = fileSystemController.getINodeAddress(pathContext->absolutePath[i]);
        auto folderName = getFolderName(nodeAddress, folderItems);
        path.push_back(folderName);
    }

    // join pomoci '/' a print do konzole
    auto delim = "/";
    auto stringStream = std::ostringstream();
    std::copy(path.begin(), path.end(), std::ostream_iterator<std::string>(stringStream, delim));
    std::cout << stringStream.str() << std::endl;
}

INode FileOperations::getRoot() {
    return fileSystemController.getRoot();
}

std::vector<FolderItem> FileOperations::getFolderItems(INode& node) {
    return fileSystemController.getFolderItems(node);
}

INode FileOperations::getINodeFromAddress(uint64_t address) {
    return fileSystemController.getINodeFromAddress(address);
}

INode FileOperations::getUpdatedINode(INode& node) {
    return fileSystemController.getUpdatedINode(node);
}

void FileOperations::getInfo(const std::string& path) {
    auto fsPath = FileSystemPath(path); // cesta
    auto itemName = fsPath.releaseBack(); // jmeno souboru

    auto absolutePath = pathContext->absolutePath;
    try {
        pathContext->moveTo(fsPath);
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException("FILE NOT FOUND (neni zdroj)");
    }
    pathContext->loadItems(); // nacteni predmetu

    auto index = pathContext->getFolderItemIndex(itemName);
    if (index == -1) {
        restorePathContextState(absolutePath);
        throw FSException("Error, specified item does not exist");
    }

    auto item = pathContext->folderItems[index];
    auto node = fileSystemController.getINodeFromAddress(item.nodeAddress);
    fileSystemController.printINodeInfo(node, item); // tisk informaci do konzole

    restorePathContextState(absolutePath);

}

void FileOperations::removeDirectory(const std::string& path) {
    auto fsPath = FileSystemPath(path);
    auto folder = fsPath.releaseBack();

    auto absolutePath = pathContext->absolutePath;

    if (folder == ".") { // nelze smazat tecku, protoze by to narusilo PathContext
        restorePathContextState(absolutePath);
        throw FSException("Failed to remove '.', invalid argument");
    }

    try {
        pathContext->moveTo(fsPath); // presun a nacteni predmetu
        pathContext->loadItems();
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException(ex.what()); // tisk chyby pri presunu
    }


    auto folderIndex = pathContext->getFolderItemIndex(folder); // zjisteni indexu slozky
    if (folderIndex == -1) {
        restorePathContextState(absolutePath);
        throw FSException("FILE NOT FOUND (neexistujici adresar)"); // adresar neexistuje
    }

    auto folderItem = pathContext->folderItems[folderIndex]; // folderItem slozky
    auto folderNode = fileSystemController.getINodeFromAddress(folderItem.nodeAddress);

    if (!folderNode.isFolder()) { // nelze smazat soubory pomoci rmdir
        restorePathContextState(absolutePath);
        throw FSException("Error, cannot remove file with \"rmdir\" command");
    }

    // slozku lze smazat pouze, kdyz v ni je '.' a '..' - tzn. 2 predmety
    if (folderNode.getFolderSize() != 2) {
        restorePathContextState(absolutePath);
        throw FSException("NOT EMPTY (adresar obsahuje podadresare, nebo soubory)");
    }

    auto parent = pathContext->absolutePath.back(); // nadrazena slozka
    fileSystemController.removeFolderItem(parent, folderItem); // odstraneni predmetu z nadrazene slozky

    if (folderNode.getRefCount() == 0) {
        fileSystemController.reclaimINode(folderNode); // vratime INode file systemu
    }

    restorePathContextState(absolutePath);
}

void FileOperations::removeFile(const std::string& path) {
    auto fsPath = FileSystemPath(path); // cesta ve file systemu
    auto fileName = fsPath.releaseBack(); // jmeno souboru

    auto absolutePath = pathContext->absolutePath;
    try {
        pathContext->moveTo(fsPath); // presun do cesty a nacteni predmetu
        pathContext->loadItems();
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException(ex.what()); // vyhozeni chyby pri presunu
    }


    auto fileIndex = pathContext->getFolderItemIndex(fileName); // zjisteni indexu souboru

    if (fileIndex == -1) {
        restorePathContextState(absolutePath);
        throw FSException("FILE NOT FOUND");
    }

    // folderItem s referenci na INode daneho souboru
    auto fileFolderItem = pathContext->folderItems[fileIndex];
    auto fileNode = fileSystemController.getINodeFromAddress(fileFolderItem.nodeAddress);

    if (fileNode.isFolder()) {
        throw FSException("Error, file is a folder, please remove it with \"rmdir\" instead");
    }

    auto parent = pathContext->absolutePath.back();
    // odstranime folder item ze slozky a vratime vsechnu pamet filesystemu
    fileSystemController.removeFolderItem(parent, fileFolderItem);

    if (fileNode.getRefCount() == 0) { // pokud je ref count = 0 odstranime soubor, jinak pouze snizime pocet referenci
        fileSystemController.reclaimINode(fileNode);
    } else {
        fileNode.decrRefCount();
        fileSystemController.refresh(fileNode); // aktualizace INode
    }

    restorePathContextState(absolutePath);
    std::cout << "OK" << std::endl;
}

void FileOperations::copyIntoFileSystem(const std::string& outPath, const std::string& path) {
    auto outputFileStream = FileStream(outPath); // stream pro cteni externiho souboru

    if (!outputFileStream.fileExists()) { // soubor neexistuje
        throw FSException("FILE NOT FOUND (neni zdroj)");
    }

    if (outputFileStream.getFileSize() > Globals::MAX_FILE_SIZE_BYTES()) { // soubor je moc velky
        throw FSException("Error, file is too large");
    }

    outputFileStream.open(); // otevreni streamu
    auto fsPath = FileSystemPath(path); // cesta ve file systemu
    auto absolutePath = pathContext->absolutePath; // backup absolutni cesty path contextu pro pozdejsi obnoveni
    auto fileName = fsPath.releaseBack(); // jmeno souboru

    try {
        pathContext->moveTo(fsPath); // presun do cesty a nacteni souboru
        pathContext->loadItems();
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException("PATH NOT FOUND (neexistuje cilova cesta)");
    }

    if (pathContext->getFolderItemIndex(fileName) != -1) {
        restorePathContextState(absolutePath);
        throw FSException("Error, file with this name already exists");
    }

    auto fileNode = fileSystemController.getFreeINode(false); // INode noveho souboru
    auto nodeAddress = fileSystemController.getINodeAddress(fileNode); // Adresa INode
    auto parentNode = pathContext->absolutePath.back(); // Rodic INode === nadrazena slozka

    try {
        FolderItem::validateFileName(fileName); // validace jmena
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException(ex.what());
    }

    auto folderItem = FolderItem(fileName, nodeAddress); // vytvoreni zaznamu pro ulozeni do slozky
    fileSystemController.appendExternalFile(parentNode, fileNode, folderItem, outputFileStream);

    restorePathContextState(absolutePath);
    std::cout << "OK" << std::endl;
}

void FileOperations::readFile(const std::string& path) {
    auto fsPath = FileSystemPath(path); // cesta ve file systemu
    auto file = fsPath.releaseBack(); // jmeno souboru

    auto absolutePath = pathContext->absolutePath; // backup absolutni cesty pro pozdejsi obnoveni
    try {
        pathContext->moveTo(fsPath); // presun a nacteni souboru
        pathContext->loadItems();
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException("FILE NOT FOUND (neni zdroj)");
    }


    auto folderItemIndex = pathContext->getFolderItemIndex(file); // index souboru ve slozce
    if (folderItemIndex == -1) {
        restorePathContextState(absolutePath);
        throw FSException("FILE NOT FOUND (neni zdroj)");
    }

    auto folderItem = pathContext->folderItems[folderItemIndex]; // zaznam o souboru
    auto nodeAddress = folderItem.nodeAddress; // adresa INode souboru
    auto node = fileSystemController.getINodeFromAddress(nodeAddress); // INode souboru

    if (node.isFolder()) { // nelze cist slozky
        restorePathContextState(absolutePath);
        throw FSException("Error, specified file is a folder");
    }

    try {
        fileSystemController.readFile(node); // precteni souboru
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException(ex.what());
    }

    restorePathContextState(absolutePath);
}

void FileOperations::exportFromFileSystem(const std::string& path, const std::string& exportPath) {
    auto filePath = exportPath.starts_with('/') || exportPath.starts_with('\\') ? exportPath.substr(1) : exportPath;

    auto outputFileStream = FileStream(filePath);
    if (outputFileStream.fileExists()) {
        throw FSException("Error, file already exists!");
    }

    outputFileStream.createFile();
    outputFileStream.openAppendOnly(); // otevre fstream v std::ios::append rezimu (protoze jiny stejne nedava smysl)

    auto fsPath = FileSystemPath(path); // cesta ve file systemu
    auto file = fsPath.releaseBack(); // nazev souboru

    auto absolutePath = pathContext->absolutePath; // absolutni cesta pro pozdejsi obnoveni
    try {
        pathContext->moveTo(fsPath); // presun do cesty a nacteni souboru
        pathContext->loadItems();
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException(ex.what());
    }

    auto folderItemIndex = pathContext->getFolderItemIndex(file); // index predmetu
    if (folderItemIndex == -1) {
        restorePathContextState(absolutePath);
        outputFileStream.deleteFile();
        throw FSException("Error, file not found");
    }

    auto folderItem = pathContext->folderItems[folderItemIndex]; // zaznam o predmetu z dane slozky
    auto nodeAddress = folderItem.nodeAddress; // adresa INode souboru
    auto node = fileSystemController.getINodeFromAddress(nodeAddress); // ziskani INode souboru

    if (node.isFolder()) { // nelze exportovat slozky
        restorePathContextState(absolutePath);
        throw FSException("Error, specified file is a folder");
    }

    try { // export souboru
        fileSystemController.exportFile(node, outputFileStream);
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException(ex.what());
    }

    restorePathContextState(absolutePath);
    std::cout << "OK" << std::endl;
}

void FileOperations::moveFile(const std::string& fileSource, const std::string& fileDest, bool deleteFromSource) {
    auto sourcePath = FileSystemPath(fileSource);
    auto destPath = FileSystemPath(fileDest);

    auto sourceFileName = sourcePath.releaseBack();
    auto destFileName = destPath.releaseBack();

    auto absolutePath = pathContext->absolutePath; // zaloha absolutni cesty path kontextu pro navrat

    // test jestli cilova cesta existuje
    try {
        pathContext->moveTo(destPath);
        pathContext->loadItems();
        if (pathContext->getFolderItemIndex(destFileName) != -1) {
            throw FSException("Error, file already exists in the target folder");
        }
    }
    catch (FSException& ex) {
        std::cout << ex.what() << std::endl;
        restorePathContextState(absolutePath);
        throw FSException("FILE NOT FOUND (neexistuje cilova cesta)");
    }
    // vratime se zpet na cestu pri zavolani funkce, jinak bychom se mohli ocitnout jinde pokud se jedna o relativni cestu
    restorePathContextState(absolutePath);

    // test jestli zdrojova cesta existuje, pokud ano, rovnou se presuneme
    try {
        pathContext->moveTo(sourcePath);
        pathContext->loadItems();
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException("FILE NOT FOUND (neni zdroj)");
    }

    // index folder itemu v dane ceste
    auto folderItemIndex = pathContext->getFolderItemIndex(sourceFileName);
    if (folderItemIndex == -1) {
        restorePathContextState(absolutePath);
        throw FSException("FILE NOT FOUND (neni zdroj)");
    }

    // ziskame dany folder item souboru, jeho inode a inode rodice (slozky)
    auto folderItem = pathContext->folderItems[folderItemIndex];
    auto fileNode = fileSystemController.getINodeFromAddress(folderItem.nodeAddress);
    auto parent = pathContext->absolutePath.back();

    if (fileNode.isFolder()) {
        restorePathContextState(absolutePath);
        throw FSException("Error, target is a folder, only files can be moved/linked");
    }

    restorePathContextState(absolutePath); // potreba vratit se zpet na puvodni path, ze ktere se prikaz volal
    pathContext->moveTo(destPath);

    auto newParent = pathContext->absolutePath.back();
    if (newParent.isFolderFull()) { // pokud je slozka plna vyhodime chybu a vratime se (nic se nevykona)
        restorePathContextState(absolutePath);
        throw FSException("Error, folder is full");
    }

    try {
        FolderItem::validateFileName(destFileName);
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException(ex.what());
    }

    auto newFolderItem = FolderItem(destFileName, folderItem.nodeAddress); // novy folder item
    fileSystemController.appendFolderItem(newParent, newFolderItem); // pridame folder item do nove slozky

    if (deleteFromSource) { // smazani z puvodniho zdroje (pro mv prikaz, pri hard linku se neprovede)

        if (newParent == parent) {
            fileSystemController.removeFolderItem(newParent, folderItem);
        } else {
            // pokud se jedna o prikaz "mv", pak odstranime z puvodni slozky, pro hardlink pouze zkopirujeme folder item
            fileSystemController.removeFolderItem(parent, folderItem); // odstranime predmet z aktualni slozky
        }

    } else {
        // jinak zvysime pocet referenci inode daneho souboru
        fileNode.incrRefCount();
        fileSystemController.refresh(fileNode); // refresh pro zapsani do souboru
    }


    std::cout << "OK" << std::endl;
    restorePathContextState(absolutePath); // vratime se zpet na puvodni cestu
}

void FileOperations::copyFile(const std::string& fileSource, const std::string& fileDest) {
    auto sourcePath = FileSystemPath(fileSource);
    auto destPath = FileSystemPath(fileDest);

    auto sourceFileName = sourcePath.releaseBack();
    auto destFileName = destPath.releaseBack();
    auto absolutePath = pathContext->absolutePath; // zaloha absolutni cesty path kontextu pro navrat

    try {
        pathContext->moveTo(destPath);
        pathContext->loadItems();

        if (pathContext->getFolderItemIndex(destFileName) != -1) {
            throw FSException("Error, file already present in the path");
        }
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException(ex.what());
    }

    // vratime se na path, ve ktere prikaz zacinal (jinak se pri relativni path ocitneme jinde)
    restorePathContextState(absolutePath);

    try {
        pathContext->moveTo(sourcePath);
        pathContext->loadItems();
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException("FILE NOT FOUND (neni zdroj)");
    }

    // index folder itemu v dane ceste
    auto folderItemIndex = pathContext->getFolderItemIndex(sourceFileName);
    if (folderItemIndex == -1) {
        restorePathContextState(absolutePath);
        throw FSException("FILE NOT FOUND (neni zdroj)");
    }

    // ziskame folder item a jeho inode
    auto folderItem = pathContext->folderItems[folderItemIndex];
    auto fileNode = fileSystemController.getINodeFromAddress(folderItem.nodeAddress);

    if (fileNode.isFolder()) {
        restorePathContextState(absolutePath);
        throw FSException("Error, target is a folder, only files can be copied");
    }

    try {
        FolderItem::validateFileName(destFileName);
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException(ex.what());
    }

    restorePathContextState(absolutePath); // opet potreba presun na path, ze ktere se prikaz
    pathContext->moveTo(destPath);
    pathContext->loadItems();

    // ziskani nove inode
    auto newFileNode = fileSystemController.getFreeINode(false);
    auto newFileNodeAddress = fileSystemController.getINodeAddress(newFileNode);
    auto newFolderItem = FolderItem(destFileName, newFileNodeAddress);
    auto newParent = pathContext->absolutePath.back();

    if (newParent.isFolderFull()) {
        fileSystemController.reclaimINode(newFileNode);
        restorePathContextState(absolutePath);
        throw FSException("Error, folder is full");
    }

    if (pathContext->getFolderItemIndex(destFileName) != -1) {
        fileSystemController.reclaimINode(newFileNode);
        restorePathContextState(absolutePath);
        throw FSException("Error, file with this name already exists");
    }

    try {
        fileSystemController.appendFolderItem(newParent, newFolderItem);
        fileSystemController.copyData(fileNode, newFileNode);
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        fileSystemController.reclaimINode(newFileNode);
        throw FSException(ex.what());
    }

    restorePathContextState(absolutePath);
    std::cout << "OK" << std::endl;
}

