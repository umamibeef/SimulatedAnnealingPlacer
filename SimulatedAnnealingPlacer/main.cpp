// C++ Includes
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
// SFML Includes
#include <SFML/Graphics.hpp>
// Program Includes
#include "SAPlacer.h"

parsedInputStruct_t *input = new parsedInputStruct_t();
placerStruct_t *placer = new placerStruct_t();

int main(int argc, char **argv)
{
    unsigned int i;
    //char * filename = argv[1];
    const char * filename = "..\\benchmarks\\apex4.txt";
    const sf::Vector2u viewportSize(
        static_cast<unsigned int>(WIN_VIEWPORT_WIDTH),
        static_cast<unsigned int>(WIN_VIEWPORT_HEIGHT));
    std::vector<sf::RectangleShape> grid;

    // Filename to read in is the second argument
    std::ifstream myfile(filename, std::ios::in);

    // Check if file was opened properly
    if(myfile.is_open())
    {
        printf("File %s opened! Here's what's in it:\n", filename);
    }
    else
    {
        printf("FATAL ERROR, file %s couldn't be opened!\n", filename);
        return -1;
    }

    // Parse input file
    parseInputFile(&myfile, input);

    // Create our render window object
    // Give it a default type (titlebar, close button, resizeable)
    sf::RenderWindow window(sf::VideoMode(
        static_cast<unsigned int>(WIN_VIEWPORT_WIDTH),
        static_cast<unsigned int>(WIN_VIEWPORT_HEIGHT)),
        "Simulated Annealing Placer", sf::Style::Default);
    window.setView(calcView(window.getSize(), viewportSize));

    // Get a grid
    grid = generateGrid(input);

    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
            if(event.type == sf::Event::Resized)
                window.setView(calcView(sf::Vector2u(event.size.width, event.size.height), viewportSize));
        }
        window.clear();

        // Draw UI
        // Draw grid
        for(i = 0; i < grid.size(); i++)
        {
            window.draw(grid[i]);
        }
        window.display();
    }

    return 0;
}

std::vector<std::string> splitString(std::string inString, char delimiter)
{
    std::vector<std::string> internal;
    std::stringstream ss(inString); // Turn the string into a stream.
    std::string temp;

    while(std::getline(ss, temp, delimiter))
    {
        internal.push_back(temp);
    }

    return internal;
}

sf::View calcView(const sf::Vector2u &windowSize, const sf::Vector2u &viewportSize)
{
    sf::FloatRect viewport(0.f, 0.f, 1.f, 1.f);
    float viewportWidth = static_cast<float>(viewportSize.x);
    float viewportHeight = static_cast<float>(viewportSize.y);
    float screenwidth = windowSize.x / static_cast<float>(viewportSize.x);
    float screenheight = windowSize.y / static_cast<float>(viewportSize.y);

    if(screenwidth > screenheight)
    {
        viewport.width = screenheight / screenwidth;
        viewport.left = (1.f - viewport.width) / 2.f;
    }
    else if(screenwidth < screenheight)
    {
        viewport.height = screenwidth / screenheight;
        viewport.top = (1.f - viewport.height) / 2.f;
    }

    sf::View view(sf::FloatRect(0.f, 0.f, viewportWidth, viewportHeight));
    view.setViewport(viewport);

    return view;
}

bool parseInputFile(std::ifstream *inputFile, parsedInputStruct_t *inputStruct)
{
    unsigned int i, j, numNodes;
    std::string line;
    std::vector<std::string> stringVec;

    // 1. Get number of cells, number of nets, and grid size
    std::getline(*inputFile, line);
    stringVec = splitString(line, ' ');
    inputStruct->numCells = stoi(stringVec[0]);
    inputStruct->numConnections = stoi(stringVec[1]);
    inputStruct->numRows = stoi(stringVec[2]);
    inputStruct->numCols = stoi(stringVec[3]);
    printf("Grid size is %d rows x %d cols\n", inputStruct->numRows, inputStruct->numCols);
    printf("Number of cells is %d\n", inputStruct->numCells);
    printf("Number of connections is %d\n", inputStruct->numConnections);

    // 2. Get all connections
    for(i = 0; i < inputStruct->numConnections; i++)
    {
        std::getline(*inputFile, line);
        stringVec = splitString(line, ' ');
        // Get number of nodes for this net
        numNodes = stoi(stringVec[0]);
        printf("Connection %d: %d nodes:\n\t", i, numNodes);
        // Now get all nodes for this net
        // Push back a new vector for this
        inputStruct->nets.push_back(std::vector<unsigned int>());
        for(j = 0; j < numNodes; j++)
        {
            inputStruct->nets[i].push_back(stoi(stringVec[j + 1]));
            printf("%d ", inputStruct->nets[i][j]);
        }
        printf("\n");
    }


    return true;
}

std::vector<sf::RectangleShape> generateGrid(parsedInputStruct_t *input)
{
    std::vector<sf::RectangleShape> grid;
    unsigned int i, j;
    float rowToColRatio, graphicportRatio, cellSize, cellOffset, cellOppositeOffset;

    // Determine the current row to column ratio
    rowToColRatio = static_cast<float>(input->numRows)/static_cast<float>(input->numCols);
    graphicportRatio = WIN_GRAPHICPORT_HEIGHT / WIN_GRAPHICPORT_WIDTH;
    
    printf("%f %f %f\n", WIN_GRAPHICPORT_HEIGHT, WIN_GRAPHICPORT_WIDTH, graphicportRatio);

    // Check which orientation gets maximized
    if(rowToColRatio > graphicportRatio)
    {
        // Use rows to fill vertically
        cellSize = WIN_GRAPHICPORT_HEIGHT / static_cast<float>(input->numRows);
        // Cell offset is always half of cell size
        cellOffset = cellSize / 2.f;
        cellOppositeOffset = cellOffset + (WIN_GRAPHICPORT_WIDTH - static_cast<float>(input->numCols) * cellSize) / 2.f;
    }
    else
    {
        // Use columns to fill horizontally
        cellSize = WIN_GRAPHICPORT_WIDTH / static_cast<float>(input->numCols);
        // Cell offset is always half of cell size
        cellOffset = cellSize / 2.f;
        cellOppositeOffset = cellOffset + (WIN_GRAPHICPORT_HEIGHT - static_cast<float>(input->numRows) * cellSize) / 2.f;
    }

    // Populate the grid vector with the data obtained above
    for(i = 0; i < input->numCols; i++)
    {
        for(j = 0; j < input->numRows; j++)
        {
            grid.push_back(sf::RectangleShape());
            if(rowToColRatio > WIN_GRAPHICPORT_HEIGHT / WIN_GRAPHICPORT_WIDTH)
            {
                grid.back().setPosition(
                    static_cast<float>((i*cellSize) + cellOppositeOffset),
                    static_cast<float>((j*cellSize) + cellOffset)
                );
            }
            else
            {
                grid.back().setPosition(
                    static_cast<float>((i*cellSize) + cellOffset),
                    static_cast<float>((j*cellSize) + cellOppositeOffset)
                );
            }
            grid.back().setSize(sf::Vector2f(cellSize * GRID_SHRINK_FACTOR, cellSize * GRID_SHRINK_FACTOR));
            grid.back().setOrigin(sf::Vector2f(cellSize * GRID_SHRINK_FACTOR * 0.5f, cellSize * GRID_SHRINK_FACTOR * 0.5f));
            grid.back().setFillColor(sf::Color::White);
        }
    }

    return grid;
}