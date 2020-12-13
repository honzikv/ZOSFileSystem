
#include "FileOperations.hpp"

FileOperations::FileOperations(FileSystemController& fileSystemController) : fileSystemController(fileSystemController),
                                                                             pathContext(fileSystemController) {

}

void FileOperations::printCurrentFolderItems() {
    for (auto& item : pathContext.folderItems) {
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
    auto absolutePath = pathContext.absolutePath; // kopie absolutni cesty

    if (fsPath.getPathType() == PathType::Absolute) {
        pathContext.moveToRoot(false);
    }

    try {
        pathContext.moveToPath(fsPath);
    }
    catch (FSException& ex) {
        std::cout << ex.what() << std::endl;
    }

    // nacteme predmety a vytiskneme
    pathContext.loadItems();
    printCurrentFolderItems();

    // vratime zpet puvodni cestu a nacteme predmety
    pathContext.absolutePath = absolutePath;
    pathContext.refresh();
}

void FileOperations::makeDirectory(const std::string& path) {
    auto fsPath = FileSystemPath(path);

    // ulozime aktualni cestu, protoze se v kontextu muzeme presunout
    auto absolutePath = pathContext.absolutePath;

    auto folderName = fsPath.back(); // nazev souboru je posledni prvek objektu FileSystemPath
    fsPath.popBack(); // odstranime nazev slozky, protoze tam se presouvat nebudeme

    if (fsPath.size() > 0) {
        // pokud je cesta vetsi nez 0, tak je potreba se nekam presunout
        if (fsPath.getPathType() == PathType::Absolute) {
            pathContext.moveToRoot(false); // presuneme se na root pokud je cesta absolutni
        }

        pathContext.moveToPath(fsPath);
    }

    pathContext.loadItems(); // nacteme soubory
    if (pathContext.folderItemExists(folderName)) {
        throw FSException("Error, specified folder/file is already present in this path");
    }

    auto parentNode = pathContext.absolutePath.back();
    auto parentNodeAddress = fileSystemController.getNodeAddress(parentNode);

    // hodi exception pokud je jmeno nesmysl a ukonci operaci
    FolderItem::validateFolderName(folderName);
    if (parentNode.isFolderFull()) {
        throw FSException("Error, specified folder is full");
    }

    auto folderNode = fileSystemController.getFreeINode(); // ziskani nove INode pro slozku
    auto folderNodeAddress = fileSystemController.getNodeAddress(folderNode);
    auto parentNodeFolderItem = FolderItem(folderName, folderNodeAddress, true);
    try {
        fileSystemController.linkFolderToParent(folderNode, folderNodeAddress, parentNodeAddress);
    }
    catch (FSException& ex) {
        throw FSException("Error, not enough space to create new folder");
    }

    try {
        fileSystemController.append(parentNode, parentNodeFolderItem);
    } catch (FSException& ex) {
        //todo clear folderNode
        fileSystemController.reclaimINode(folderNode);
        throw FSException("Error, not enough space to create new folder");
    }

    pathContext.absolutePath = absolutePath;
    pathContext.refresh();
}


