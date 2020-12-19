
#include "FileOperations.hpp"

FileOperations::FileOperations(FileSystemController& fileSystemController) : fileSystemController(
        fileSystemController) {
    this->pathContext = std::make_unique<PathContext>(*this);
}

void FileOperations::printCurrentFolderItems() {
    for (auto& item : pathContext->folderItems) {
        std::cout << item.getItemName() << std::endl;
    }
}

void FileOperations::listItems(const std::string& path) {
    auto fsPath = FileSystemPath(path);

    if (fsPath.getPathType() == PathType::Empty) {
        printCurrentFolderItems();
        return;
    }

    // Pro zjisteni predmetu jinde nez v aktualni slozce potrebujeme zmenit absolutePath,
    // tzn je potreba vytvorit kopii, kterou pote zpetne nahradime aby se uzivatel nevyskytl na miste, ktere chtel
    // zobrazit misto toho, kde se nachazel
    auto absolutePath = pathContext->absolutePath; // kopie absolutni cesty

    if (fsPath.getPathType() == PathType::Absolute) {
        pathContext->moveToRoot(false);
    }

    try {
        pathContext->moveTo(fsPath);
    }
    catch (FSException& ex) {
        std::cout << ex.what() << std::endl;
        return;
    }

    // nacteme predmety a vytiskneme
    pathContext->loadItems();
    printCurrentFolderItems();

    // vratime zpet puvodni cestu a nacteme predmety
    pathContext->absolutePath = absolutePath;
    pathContext->refresh();
}

void FileOperations::makeDirectory(const std::string& path) {
    auto fsPath = FileSystemPath(path);

    // ulozime aktualni cestu, protoze se v kontextu muzeme presunout
    auto absolutePath = pathContext->absolutePath;

    auto folderName = fsPath.back(); // nazev souboru je posledni prvek objektu FileSystemPath
    fsPath.popBack(); // odstranime nazev slozky, protoze tam se presouvat nebudeme

    if (fsPath.size() > 0) {
        // pokud je cesta vetsi nez 0, tak je potreba se nekam presunout
        if (fsPath.getPathType() == PathType::Absolute) {
            pathContext->moveToRoot(false); // presuneme se na root pokud je cesta absolutni
        }

        pathContext->moveTo(fsPath); // presuneme se do aktualni cesty
    }

    pathContext->loadItems(); // nacteme soubory
    if (pathContext->folderItemExists(folderName)) {
        throw FSException("Error, specified folder/file is already present in this path");
    }

    auto parentNode = pathContext->absolutePath.back();
    auto parentNodeAddress = fileSystemController.getINodeAddress(parentNode);

    // hodi exception pokud je jmeno nesmysl a ukonci operaci
    FolderItem::validateFolderName(folderName);
    if (parentNode.isFolderFull()) {
        throw FSException("Error, specified folder is full");
    }

    auto folderNode = fileSystemController.getFreeINode(true); // ziskani nove INode pro slozku
    auto folderNodeAddress = fileSystemController.getINodeAddress(folderNode); // ziskani adresy
    auto parentNodeFolderItem = FolderItem(folderName, folderNodeAddress, true);

    try {
        fileSystemController.linkFolderToParent(folderNode, folderNodeAddress, parentNodeAddress);
    }
    catch (FSException& ex) {
        throw FSException("Error, not enough space to create new folder");
    }

    try {
        fileSystemController.appendFolder(parentNode, parentNodeFolderItem);
    } catch (FSException& ex) {
        //todo clear folderNode
        //  fileSystemController.reclaimINode(folderNode);
        throw FSException("Error, not enough space to create new folder");
    }

    restorePathContextState(absolutePath);
}

void FileOperations::restorePathContextState(const std::vector<INode>& absolutePath) {
    pathContext->absolutePath = absolutePath;
    pathContext->refresh();
}

void FileOperations::changeDirectory(const std::string& path) {
    auto fsPath = FileSystemPath(path);
    auto absolutePath = pathContext->absolutePath; // kopie pro pripad ze se zmena slozky nezdari

    if (fsPath.getPathType() == PathType::Absolute) {
        pathContext->moveToRoot();
    }

    try {
        pathContext->moveTo(fsPath);
    }
    catch (FSException& ex) {
        std::cout << ex.what() << std::endl;
        pathContext->absolutePath = absolutePath;
    }

    pathContext->refresh();
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
    if (path.empty()) {
        auto node = pathContext->absolutePath.back();
        fileSystemController.printINodeInfo(node);
        return;
    }

    auto fsPath = FileSystemPath(path);
    auto itemName = fsPath.releaseBack();

    auto absolutePath = pathContext->absolutePath;
    if (fsPath.size() > 0) {
        pathContext->moveTo(fsPath);
        pathContext->loadItems();
    }

    auto index = pathContext->getFolderItemIndex(itemName);
    if (index == -1) {
        throw FSException("Error, specified item does not exist");
    }

    auto item = pathContext->folderItems[index];
    auto node = fileSystemController.getINodeFromAddress(item.nodeAddress);
    fileSystemController.printINodeInfo(node);

    if (fsPath.size() > 0) {
        restorePathContextState(absolutePath);
    }
}

void FileOperations::removeDirectory(const std::string& path) {
    auto fsPath = FileSystemPath(path);
    auto folder = fsPath.releaseBack();

    auto absolutePath = pathContext->absolutePath;
    if (fsPath.size() > 0) {
        pathContext->moveTo(fsPath);
        pathContext->loadItems();
    }

    auto folderIndex = pathContext->getFolderItemIndex(folder);
    if (folderIndex == -1) {
        throw FSException("Error, folder does not exist in this path");
    }

    auto folderItem = pathContext->folderItems[folderIndex];
    auto folderNode = fileSystemController.getINodeFromAddress(folderItem.nodeAddress);

    if (!folderNode.isFolder()) {
        throw FSException("Error, cannot remove file with \"rmdir\" command");
    }

    if (folderNode.getRefCount() != 0) {
        throw FSException("Error, folder is not empty");
    }

    auto parent = pathContext->absolutePath.back();
    fileSystemController.removeFolderItem(parent, folderItem);
    fileSystemController.reclaimINode(folderNode);

    restorePathContextState(absolutePath);
}

void FileOperations::removeFile(const std::string& path) {
    auto fsPath = FileSystemPath(path);
    auto fileName = fsPath.releaseBack();

    auto absolutePath = pathContext->absolutePath;
    if (fsPath.size() > 0) {
        pathContext->moveTo(fsPath);
        pathContext->loadItems();
    }

    auto fileIndex = pathContext->getFolderItemIndex(fileName);

    if (fileIndex == -1) {
        throw FSException("Error, no such found found");
    }

    // folderItem s referenci na INode daneho souboru
    auto fileFolderItem = pathContext->folderItems[fileIndex];
    auto fileNode = fileSystemController.getINodeFromAddress(fileFolderItem.nodeAddress);

    if (fileNode.isFolder()) {
        throw FSException("Error, file is a folder, please remove it with \"rmdir\" instead");
    }

    auto parent = pathContext->absolutePath.back();
    fileSystemController.removeFolderItem(parent, fileFolderItem);
    fileSystemController.reclaimINode(fileNode);

    restorePathContextState(absolutePath);
}

void FileOperations::copyIntoFileSystem(const std::string& outPath, const std::string& path) {
    auto outputFileStream = FileStream(outPath); // stream pro cteni externiho souboru

    if (!outputFileStream.fileExists()) {
        throw FSException("Error, specified file does not exist!");
    }

    if (outputFileStream.getFileSize() > Globals::MAX_FILE_SIZE_BYTES()) {
        throw FSException("Error, file is too large");
    }

    outputFileStream.open();
    auto fsPath = FileSystemPath(path);
    auto absolutePath = pathContext->absolutePath;
    auto fileName = fsPath.releaseBack();

    if (fsPath.size() > 0) {
        pathContext->moveTo(fsPath);
        pathContext->loadItems();
    }

    if (pathContext->getFolderItemIndex(fileName) != -1) {
        restorePathContextState(absolutePath);
        throw FSException("Error, file with this name already exists");
    }

    auto fileNode = fileSystemController.getFreeINode(false);
    auto nodeAddress = fileSystemController.getINodeAddress(fileNode);
    auto parentNode = pathContext->absolutePath.back();

    try {
        FolderItem::validateFileName(fileName);
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException(ex.what());
    }

    auto folderItem = FolderItem(fileName, nodeAddress, false);
    fileSystemController.appendFile(parentNode, fileNode, folderItem, outputFileStream);

    restorePathContextState(absolutePath);
}

void FileOperations::readFile(const std::string& path) {
    auto fsPath = FileSystemPath(path);
    auto file = fsPath.releaseBack();

    auto absolutePath = pathContext->absolutePath;
    if (fsPath.size() > 0) {
        pathContext->moveTo(fsPath);
        pathContext->loadItems();
    }

    auto folderItemIndex = pathContext->getFolderItemIndex(file);
    if (folderItemIndex == -1) {
        throw FSException("Error, file not found");
    }

    auto folderItem = pathContext->folderItems[folderItemIndex];
    auto nodeAddress = folderItem.nodeAddress;
    auto node = fileSystemController.getINodeFromAddress(nodeAddress);

    if (node.isFolder()) {
        throw FSException("Error, specified file is a folder");
    }

    try {
        fileSystemController.readFile(node);
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException(ex.what());
    }

    if (fsPath.size() > 0) {
        restorePathContextState(absolutePath);
    }
}

void FileOperations::exportFromFileSystem(const std::string& path, const std::string& exportPath) {
    auto outputFileStream = FileStream(exportPath);

    if (outputFileStream.fileExists()) {
        throw FSException("Error, file already exists!");
    }

    outputFileStream.createFile();
    outputFileStream.openAppendOnly();

    auto fsPath = FileSystemPath(path);
    auto file = fsPath.releaseBack();

    auto absolutePath = pathContext->absolutePath;
    if (fsPath.size() > 0) {
        pathContext->moveTo(fsPath);
        pathContext->loadItems();
    }

    auto folderItemIndex = pathContext->getFolderItemIndex(file);
    if (folderItemIndex == -1) {
        throw FSException("Error, file not found");
    }

    auto folderItem = pathContext->folderItems[folderItemIndex];
    auto nodeAddress = folderItem.nodeAddress;
    auto node = fileSystemController.getINodeFromAddress(nodeAddress);

    if (node.isFolder()) {
        throw FSException("Error, specified file is a folder");
    }

    try {
        fileSystemController.exportFile(node, outputFileStream);
    }
    catch (FSException& ex) {
        restorePathContextState(absolutePath);
        throw FSException(ex.what());
    }

    if (fsPath.size() > 0) {
        restorePathContextState(absolutePath);
    }
}

