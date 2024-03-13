#include "exo-vtk-include.h"
#include "config.h"
#include "helpers.h"


//*  //Enlever le premier slash pour commenter
/*#define FICHIER  "Frog_CHAR_X_256_Y_256_Z_44.raw"

int gridSize = 256;
int YgridSize = 256;
int ZgridSize = 44;

#define CHAR
#define SMALL

int startexploreval=13;
int endexploreval=20;//*/


 //Ajouter un slash pour décommenter
 #define FICHIER  "Mystere1_SHORT_X_512_Y_512_Z_134.raw"
 int gridSize = 512;
 int YgridSize = 512;
 int ZgridSize = 134;

 #define SHORT
 #define SMALL

 int startexploreval=16500;
 int endexploreval=65000;//


/*
 #define FICHIER  "Mystere5_SHORT_X_2048_Y_2048_Z_756.raw"

 int gridSize = 2048;
 int YgridSize = 2048;
 int ZgridSize = 756;

 #define SHORT
 #define BIG

 int startexploreval=100;
 int endexploreval=65000;//*/

/*
 #define FICHIER  "Mystere6_CHAR_X_1118_Y_2046_Z_694.raw"

 int gridSize = 1118;
 int YgridSize = 2046;
 int ZgridSize = 694;

 #define CHAR
 #define BIG

 int startexploreval=1;
 int endexploreval=255;//*/


/*
 #define FICHIER  "Mystere4_SHORT_X_512_Y_512_Z_322.raw"
 int gridSize = 512;
 int YgridSize = 512;
 int ZgridSize = 322;

 #define SHORT
 #define SMALL

 int startexploreval=1;
 int endexploreval=65000; //*/


int winSize = 500;

const char *prefix = "";
const char *location = FICHIER ;
int passNum = 0;
int nbImageIntermediaire = 16;
int imageMax = 5;

using std::cerr;
using std::endl;

// Function prototypes
vtkRectilinearGrid  *ReadGrid(int zStart, int zEnd);
void WriteImage(const char *name, const float *rgba, int width, int height);
bool ComposeImageZbuffer(float *rgba_out, float *zbuffer,   int image_width, int image_height);


int main(int argc, char *argv[])
{
    // Initialisation du générateur de nombres aléatoires
    srand ( time(NULL) );

    // Déclaration de la grille VTK qui contiendra les données lues
    vtkRectilinearGrid *reader = NULL;

    // Mesure de l'utilisation de la mémoire au début du programme
    GetMemorySize("initialization");
    int t1;
    t1 = timer->StartTimer(); // Démarrage d'un chronomètre pour mesurer le temps d'exécution

    for (int countImage = 0; countImage < imageMax; countImage++)
    {
        // Création d'une fenêtre de visualisation de taille winSize x winSize
        int npixels = winSize * winSize;

        // Tableaux pour stocker les pixels de couleur et de profondeur
        float *auxrgba = new float[4 * npixels]; // RGBA pour chaque pixel
        float *auxzbuffer = new float[npixels]; // Z-buffer pour chaque pixel

        // Initialisation des tableaux auxrgba et auxzbuffer
        for (int i = 0 ; i < npixels ; i++){
            auxzbuffer[i] = 1.0; // Profondeur initiale maximale
            auxrgba[i*4] = 0; // Couleur initiale rouge
            auxrgba[i*4+1] = 0; // Couleur initiale verte
            auxrgba[i*4+2] = 0; // Couleur initiale bleue
            auxrgba[i*4+3] = 0; // Couleur initiale alpha
        }

        // Calcul de l'intervalle de valeurs d'isovaleur et du pas pour les images intermédiaires
        int range = (endexploreval - startexploreval);
        int stepImage = range / imageMax;
        int startValueImage = startexploreval + stepImage * countImage;

        // Création d'une nouvelle table de couleurs (Lookup Table)
        vtkLookupTable *lut = vtkLookupTable::New();
        lut->Build();

        // Filtre pour créer des surfaces isovaleurs (contours) à partir des données volumétriques
        vtkContourFilter *cf = vtkContourFilter::New();
        cf->SetNumberOfContours(1);
        cf->SetValue(0, 1); // Définition de la première isovaleur

        // Transformation pour ajuster les dimensions des données à la fenêtre de visualisation
        vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
        transform->Scale(gridSize / (float) maxsize, YgridSize / (float) maxsize, ZgridSize / (float) maxsize);

        // Application de la transformation au filtre de contour
        vtkSmartPointer<vtkTransformFilter> transformFilter = vtkSmartPointer<vtkTransformFilter>::New();
        transformFilter->SetInputConnection(cf->GetOutputPort());
        transformFilter->SetTransform(transform);

        // Mappage des données transformées à l'acteur (objet graphique) pour le rendu
        vtkDataSetMapper *mapper = vtkDataSetMapper::New();
        mapper->SetInputConnection(transformFilter->GetOutputPort());

        // Acteur qui va être rendu dans la fenêtre graphique
        vtkActor *actor = vtkActor::New();
        actor->SetMapper(mapper);

        // Réglage de l'échelle des couleurs et de la table de couleurs pour le mapper
        mapper->SetScalarRange(0, nbImageIntermediaire);
        mapper->SetLookupTable(lut);

        // Rendu de l'acteur
        vtkRenderer *ren = vtkRenderer::New();
        ren->AddActor(actor);
        ren->SetViewport(0, 0, 1, 1);

        // Fenêtre de rendu hors écran
        vtkRenderWindow *renwin = vtkRenderWindow::New();
        renwin->OffScreenRenderingOn();
        renwin->SetSize(winSize, winSize);
        renwin->AddRenderer(ren);

        // Configuration de la caméra pour la scène 3D
        vtkCamera *cam = ren->GetActiveCamera();
        cam->SetFocalPoint(0.5, 0.5, 0.5);
        cam->SetPosition(0.5, 0.5, 3.);
        cam->SetViewUp(0., -1.0, 0.0);
        cam->Elevation(45);

        // Début d'une boucle pour générer et enregistrer les images intermédiaires basées sur la décomposition de l'espace Z (profondeur)
        for (passNum = 0; passNum < nbImageIntermediaire; passNum++) {
            // Calcul de l'intervalle de profondeur Z à traiter pour cette image intermédiaire
            int step = (ZgridSize / nbImageIntermediaire);
            int zStart = passNum * step; // Début de l'intervalle Z pour cette passe
            int zEnd = zStart + step; // Fin de l'intervalle Z pour cette passe
            if(zEnd >= ZgridSize) zEnd = ZgridSize - 1; // S"assurer qu'on ne dépasse pas la limite de Z

            // Vérification de l'utilisation de la mémoire avant de lire la grille
            GetMemorySize(("Pass " + std::to_string(nbImageIntermediaire) + " before read").c_str());

            // Lecture de la portion de grille correspondant à l'intervalle Z actuel
            vtkRectilinearGrid *rg = ReadGrid(zStart, zEnd);

            // Vérification de l'utilisation de la mémoire après la lecture de la grille
            GetMemorySize(("Pass " + std::to_string(passNum) + " after read").c_str());

            // Configuration de l'entrée du filtre de contour avec les données lues
            cf->SetInputData(rg);
            rg->Delete(); // Suppression de l'objet de grille pour libérer la mémoire

            // Réglage de la valeur d'isovaleur pour la création de surfaces
            cf->SetValue(0, startValueImage);
            cf->Update(); // Mise à jour du filtre pour appliquer les modifications
            cf->GetOutput()->GetPointData()->SetActiveScalars("pass_num");

            // Rendu de la scène dans la fenêtre de rendu
            renwin->Render();

            // Récupération des données de pixels colorés et de profondeur de la fenêtre de rendu
            float *rgba = renwin->GetRGBAPixelData(0, 0, winSize - 1, winSize - 1, 1);
            float *zbuffer = renwin->GetZbufferData(0, 0, winSize - 1, winSize - 1);

            // Composition de l'image finale en utilisant le z-buffer pour conserver le pixel le plus proche
            for (int i = 0; i < winSize * winSize; i++){
                if (auxzbuffer[i] > zbuffer[i]) {
                    auxzbuffer[i] = zbuffer[i]; // Mise à jour du z-buffer auxiliaire
                    // Mise à jour des pixels colorés auxiliaires avec les valeurs de rgba actuelles
                    auxrgba[i*4] = rgba[i*4];
                    // Répéter pour les canaux vert, bleu et alpha
                }
            }

            // Création du nom de fichier et enregistrement de l'image intermédiaire actuelle
            char name[128];
            sprintf(name, "imageInter%d-%d.png", countImage, passNum);
            WriteImage(name, rgba, winSize, winSize);

            // Composition et enregistrement de l'image basée sur le z-buffer
            float *new_rgba = new float[4 * npixels];
            bool didComposite = ComposeImageZbuffer(new_rgba, zbuffer, winSize, winSize);
            char namez[128];
            sprintf(namez, "image%d-%dZ.png", countImage, passNum);
            WriteImage(namez, new_rgba, winSize, winSize);

            // Libération de la mémoire des tableaux rgba et zbuffer
            free(rgba);
            free(zbuffer);
            free(new_rgba);
        }
        // Création du nom de fichier et enregistrement de l'image finale pour cette passe
        char finalName[128];
        sprintf(finalName, "imageFinal%d-%d.png", countImage, startValueImage);
        WriteImage(finalName, auxrgba, winSize, winSize);

        // Nettoyage des objets VTK après utilisation
        mapper->Delete();
        cf->Delete();
        ren->RemoveActor(actor);
        actor->Delete();
        ren->Delete();
        renwin->Delete();

    }
    GetMemorySize("end");

    // Arrêt du chronomètre et affichage du temps écoulé
    timer->StopTimer(t1,"time");
}



// You should not need to modify these routines.

vtkRectilinearGrid *
ReadGrid(int zStart, int zEnd)
{
    int  i;
    std::string file=(MY_DATA_PATH+ (std::string )FICHIER);

    ifstream ifile(file.c_str());
    if (ifile.fail())
    {
        cerr << prefix << "Unable to open file: " << MY_DATA_PATH+ (std::string )FICHIER<< "!!" << endl;
        throw std::runtime_error("can't find the file!! Check the name and the path of this file? ");
    }

    cerr << prefix << "Reading from " << zStart << " to " << zEnd << endl;

    vtkRectilinearGrid *rg = vtkRectilinearGrid::New();
    vtkFloatArray *X = vtkFloatArray::New();
    X->SetNumberOfTuples(gridSize);
    for (i = 0 ; i < gridSize ; i++)
        X->SetTuple1(i, i/(gridSize-1.0));
    rg->SetXCoordinates(X);
    X->Delete();
    vtkFloatArray *Y = vtkFloatArray::New();
    Y->SetNumberOfTuples(YgridSize);
    for (i = 0 ; i < YgridSize ; i++)
        Y->SetTuple1(i, i/(YgridSize-1.0));
    rg->SetYCoordinates(Y);
    Y->Delete();
    vtkFloatArray *Z = vtkFloatArray::New();
    int numSlicesToRead = zEnd-zStart+1;
    Z->SetNumberOfTuples(numSlicesToRead);
    for (i = zStart ; i <= zEnd ; i++)
        Z->SetTuple1(i-zStart, i/(ZgridSize-1.0));
    rg->SetZCoordinates(Z);
    Z->Delete();

    rg->SetDimensions(gridSize, YgridSize, numSlicesToRead);

    unsigned int valuesPerSlice  = gridSize*YgridSize;

#if defined(SHORT)
    unsigned int bytesPerSlice   = sizeof(short)*valuesPerSlice;

#elif defined(CHAR)
    unsigned int bytesPerSlice   = sizeof(char)*valuesPerSlice;

#elif  defined(FLOAT)
    unsigned int bytesPerSlice   = sizeof(float)*valuesPerSlice;

#else
#error Unsupported choice setting
#endif


#if defined(SMALL)
    unsigned int offset          = (unsigned int)zStart * (unsigned int)bytesPerSlice;
    unsigned int bytesToRead     = bytesPerSlice*numSlicesToRead;
    unsigned int valuesToRead    = valuesPerSlice*numSlicesToRead;
#elif defined(BIG)
    unsigned long long offset          = (unsigned long long)zStart * bytesPerSlice;
    unsigned long long  bytesToRead     = (unsigned long long )bytesPerSlice*numSlicesToRead;
    unsigned int valuesToRead    = (unsigned int )valuesPerSlice*numSlicesToRead;
#else
#error Unsupported choice setting
#endif



#if defined(SHORT)
    vtkUnsignedShortArray *scalars = vtkUnsignedShortArray::New();
    scalars->SetNumberOfTuples(valuesToRead);
    unsigned short *arr = scalars->GetPointer(0);

#elif defined(CHAR)
    vtkUnsignedCharArray *scalars = vtkUnsignedCharArray::New();
    scalars->SetNumberOfTuples(valuesToRead);
    unsigned char *arr = scalars->GetPointer(0);

#elif  defined(FLOAT)
    vtkFloatArray *scalars = vtkFloatArray::New();
    scalars->SetNumberOfTuples(valuesToRead);
    float *arr = scalars->GetPointer(0);
#else
#error Unsupported choice setting
#endif





    ifile.seekg(offset, ios::beg);
    ifile.read((char *)arr, bytesToRead);
    ifile.close();

    int min=+2147483647;
    int max =0;

#if defined(SMALL)
    for (unsigned int i = 0 ; i < valuesToRead ; i++){
#elif defined(BIG)
        for (unsigned long long int i = 0 ; i < valuesToRead ; i++){
#endif

            if (min>(scalars->GetPointer(0))[i]) min=(scalars->GetPointer(0))[i];
            if (max<(scalars->GetPointer(0))[i]) max=(scalars->GetPointer(0))[i];

            if(rand()%(valuesToRead/20)==0){
#if defined(SHORT)
                std::cout<<(unsigned short)(scalars->GetPointer(0))[i]<<" ";
#elif defined(CHAR)
                std::cout<<(unsigned short)(scalars->GetPointer(0))[i]<<" ";
#elif  defined(FLOAT)
                std::cout<<(float)(scalars->GetPointer(0))[i]<<" ";
#else
#error Unsupported choice setting
#endif

            }
        }



        std::cout<<"min value read: "<<min<<endl;
        std::cout<<"max value read: "<<max<<endl;
        std::fflush(stdout);


        scalars->SetName("entropy");
        rg->GetPointData()->AddArray(scalars);
        scalars->Delete();

        vtkFloatArray *pr = vtkFloatArray::New();
        pr->SetNumberOfTuples(valuesToRead);
#if defined(SMALL)
        for (unsigned int i = 0 ; i < valuesToRead ; i++)
#elif defined(BIG)
            for (unsigned long long  i = 0 ; i < valuesToRead ; i++)
#endif
                pr->SetTuple1(i, passNum);

        pr->SetName("pass_num");
        rg->GetPointData()->AddArray(pr);
        pr->Delete();

        rg->GetPointData()->SetActiveScalars("entropy");

        cerr << file << " Done reading" << endl;
        return rg;
    }



    void
    WriteImage(const char *name, const float *rgba, int width, int height)
    {
        vtkImageData *img = vtkImageData::New();
        img->SetDimensions(width, height, 1);
#if VTK_MAJOR_VERSION <= 5
        img->SetNumberOfScalarComponents(3);
        img->SetScalarTypeToUnsignedChar();
#else
        img->AllocateScalars(VTK_UNSIGNED_CHAR,3);
#endif

        for (int i = 0 ; i < width ; i++)
            for (int j = 0 ; j < height ; j++)
            {
                unsigned char *ptr = (unsigned char *) img->GetScalarPointer(i, j, 0);
                int idx = j*width + i;
                ptr[0] = (unsigned char) (255*rgba[4*idx + 0]);
                ptr[1] = (unsigned char) (255*rgba[4*idx + 1]);
                ptr[2] = (unsigned char) (255*rgba[4*idx + 2]);
            }


        vtkPNGWriter *writer = vtkPNGWriter::New();
        writer->SetInputData(img);
        writer->SetFileName(name);
        writer->Write();

        img->Delete();
        writer->Delete();
    }


    bool ComposeImageZbuffer(float *rgba_out, float *zbuffer,   int image_width, int image_height)
    {
        int npixels = image_width*image_height;

        float min=1;
        float max=0;
        for (int i = 0 ; i < npixels ; i++){
            if (zbuffer[i]<min) min=zbuffer[i];
            if (zbuffer[i]>max) max=zbuffer[i];

        }
        std::cout<<"min:"<<min;
        std::cout<<"max:"<<max<<"  ";

        float coef=1.0/((max-min));

        std::cout<<"coef:"<<coef<<"  ";

        for (int i = 0 ; i < npixels ; i++){

            rgba_out[i*4] = (zbuffer[i]==1.0?0:1-coef*(zbuffer[i]-min));
            rgba_out[i*4+1] = (zbuffer[i]==1.0?0:1-coef*(zbuffer[i]-min));
            rgba_out[i*4+2] = (zbuffer[i]==1.0?0:1-coef*(zbuffer[i]-min));
            rgba_out[i*4+3] = 0.0;
        }


        return false;
    }

